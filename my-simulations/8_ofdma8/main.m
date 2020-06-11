fclose('all');

tcpR = {'0', '1'};
topoR = {'4'};
mcsR = {'4'};
sizeR = {'1e2', '2e2', '5e2', '10e2'};
rateR = {'10e5', '15e5', '20e5', '25e5', '30e5'};
kR = 1:2;

for tcpI = 1:length(tcpR)
    tcp = tcpR{tcpI};
    for topoI = 1:length(topoR)
        topo = topoR{topoI};
        for mcsI = 1:length(mcsR)
            mcs = mcsR{mcsI};
            for sizeI = 1:length(sizeR)
                size = sizeR{sizeI};
                thr = zeros(2, length(rateR), length(kR));
                delay = zeros(2, length(rateR), length(kR));
                loss = zeros(2, length(rateR), length(kR));
                for rateI = 1:length(rateR)
                    rate = rateR{rateI};
                    for k = kR
                        % csma
                        fileName = ['csma-topo', topo, '-mcs', mcs, ...
                            '-rate', rate, '-size', size, '-tcp', tcp, ...
                            '-', num2str(k), '.txt'];
                        fileId = fopen(fileName);
                        A = textscan(fileId, '%s %f %f %f %f', ...
                            'HeaderLines', 67, 'Delimiter', '\t');
                        A = cell2mat(A(2:5));
                        A = A(1:3,:);
                        thr(1, rateI, k) = mean(A(1,:));
                        delay(1, rateI, k) = mean(A(2,:));
                        loss(1, rateI, k) = mean(A(3,:));
                        fclose(fileId);
                        % ofdma
                        fileName = ['ofdma-topo', topo, '-mcs', mcs, ...
                            '-rate', rate, '-size', size, '-tcp', tcp, ...
                            '-', num2str(k), '.txt'];
                        fileId = fopen(fileName);
                        A = textscan(fileId, '%s %f %f %f %f', ...
                            'HeaderLines', 72, 'Delimiter', '\t');
                        A = cell2mat(A(2:5));
                        A = A(1:3,:);
                        thr(2, rateI, k) = mean(A(1,:));
                        delay(2, rateI, k) = sum(A(1,:).*A(2,:)) / sum(A(1,:));
                        loss(2, rateI, k) = sum(A(1,:).*A(3,:)) / sum(A(1,:));
                        fclose(fileId);
                    end
                end
                
                figure;
                box on; hold on; grid on;
                scatter3(reshape(thr(1,:,:), 1, []), ...
                    reshape(delay(1,:,:), 1, []), ...
                    reshape(loss(1,:,:), 1, []), ...
                    'rx', 'LineWidth', 2);
                scatter3(reshape(thr(2,:,:), 1, []), ...
                    reshape(delay(2,:,:), 1, []), ...
                    reshape(loss(2,:,:), 1, []), ...
                    'b+', 'LineWidth', 2);
                legend({'csma', 'ofdma'}, 'Location', 'northeast');
                xlabel('throughput (Kbps)');
                ylabel('delay (ms)');
                zlabel('loss percentage');
                title(['tcp', tcp, '-topo', topo, '-mcs', mcs, '-size', size]);
                close;
            end
        end
    end
end