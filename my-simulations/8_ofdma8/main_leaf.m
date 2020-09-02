fclose('all');

tcpR = {'0'};
topoR = {'4', '22', '23', '24', '33', '222'};
mcsR = {'3', '4', '5'};
psizeR = {'4e3'};
rateR = {'5e6', '10e6', '15e6', '20e6', '25e6', '30e6', '35e6', '40e6'};
kR = 1:50;

folderName = 'onoffmodify';

for tcpI = 1:length(tcpR)
    tcp = tcpR{tcpI};
    for topoI = 1:length(topoR)
        topo = topoR{topoI};
        %%%
        totalN = 1;
        totalL = 1;
        for i = length(topo):-1:1
            totalN = totalN * str2double(topo(i)) + 1;
            totalL = totalL * str2double(topo(i));
        end
        scanForm = ['%s', repmat(' %f', [1, totalN-1])];
        %%%
        for mcsI = 1:length(mcsR)
            mcs = mcsR{mcsI};
            for sizeI = 1:length(psizeR)
                psize = psizeR{sizeI};
                thr = nan(2, length(rateR), length(kR));
                delay = nan(2, length(rateR), length(kR));
                loss = nan(2, length(rateR), length(kR));
                for rateI = 1:length(rateR)
                    rate = rateR{rateI};
                    for k = kR
                        % csma
                        fileName = [folderName, ...
                            '/topo', topo, '-mcs', mcs, '-size', psize, ...
                            '/csma-topo', topo, '-mcs', mcs, ...
                            '-rate', rate, '-size', psize, '-tcp', tcp, ...
                            '-', num2str(k), '.txt'];
                        fileId = fopen(fileName);
                        A = textscan(fileId, scanForm, 3, ...
                            'HeaderLines', 37, 'Delimiter', '\t');
                        fclose(fileId);
                        try
                            A = cell2mat(A(end-totalL+1:end));
                            thr(1, rateI, k) = mean(A(1,:));
                            delay(1, rateI, k) = A(2,:) * A(1,:).' / sum(A(1,:));
                            loss(1, rateI, k) = A(3,:) * A(1,:).' / sum(A(1,:));
                        catch
                        end
                        % ofdma
                        fileName = [folderName, ...
                            '/topo', topo, '-mcs', mcs, '-size', psize, ...
                            '/ofdma-topo', topo, '-mcs', mcs, ...
                            '-rate', rate, '-size', psize, '-tcp', tcp, ...
                            '-', num2str(k), '.txt'];
                        fileId = fopen(fileName);
                        A = textscan(fileId, scanForm, 3, ...
                            'HeaderLines', 37+totalN, 'Delimiter', '\t');
                        fclose(fileId);
                        try
                            A = cell2mat(A(end-totalL+1:end));
                            thr(2, rateI, k) = mean(A(1,:));
                            delay(2, rateI, k) = A(2,:) * A(1,:).' / sum(A(1,:));
                            loss(2, rateI, k) = A(3,:) * A(1,:).' / sum(A(1,:));
                        catch
                        end
                    end
                end
                
                %% figure
                figure('Position', [300 300 1600 900]);
                %%% scatter
                subplot(2,3,1);
                box on; hold on; grid on;
                scatter(reshape(thr(1,:,:), 1, []), ...
                    reshape(delay(1,:,:), 1, []), ...
                    'rx', 'LineWidth', 2);
                scatter(reshape(thr(2,:,:), 1, []), ...
                    reshape(delay(2,:,:), 1, []), ...
                    'b+', 'LineWidth', 2);
                legend({'csma', 'ofdma'}, 'Location', 'northwest');
                xlabel('throughput (Kbps)');
                ylabel('delay (ms)');
                title(['tcp', tcp, '-topo', topo, '-mcs', mcs, '-size', psize]);
                subplot(2,3,2);
                box on; hold on; grid on;
                scatter(reshape(thr(1,:,:), 1, []), ...
                    reshape(loss(1,:,:), 1, []), ...
                    'rx', 'LineWidth', 2);
                scatter(reshape(thr(2,:,:), 1, []), ...
                    reshape(loss(2,:,:), 1, []), ...
                    'b+', 'LineWidth', 2);
                legend({'csma', 'ofdma'}, 'Location', 'northwest');
                ylim([0 1]);
                xlabel('throughput (Kbps)');
                ylabel('loss');
                %%% plot
                subplot(2,3,4);
                box on; hold on; grid on;
                plot(sscanf(sprintf('%s ', rateR{:}), '%f'), ...
                    nanmedian(thr(1,:,:), 3), ...
                    'rx-', 'LineWidth', 2);
                plot(sscanf(sprintf('%s ', rateR{:}), '%f'), ...
                    nanmedian(thr(2,:,:), 3), ...
                    'b+-', 'LineWidth', 2);
                legend({'csma', 'ofdma'}, 'Location', 'northwest');
                xlabel('inject throughput (Kbps)');
                ylabel('throughput (Kbps)');
                title(['tcp', tcp, '-topo', topo, '-mcs', mcs, '-size', psize]);
                subplot(2,3,5);
                box on; hold on; grid on;
                plot(sscanf(sprintf('%s ', rateR{:}), '%f'), ...
                    nanmedian(delay(1,:,:), 3), ...
                    'rx-', 'LineWidth', 2);
                plot(sscanf(sprintf('%s ', rateR{:}), '%f'), ...
                    nanmedian(delay(2,:,:), 3), ...
                    'b+-', 'LineWidth', 2);
                legend({'csma', 'ofdma'}, 'Location', 'northwest');
                xlabel('inject throughput (Kbps)');
                ylabel('delay (ms)');
                subplot(2,3,6);
                box on; hold on; grid on;
                plot(sscanf(sprintf('%s ', rateR{:}), '%f'), ...
                    nanmedian(loss(1,:,:), 3), ...
                    'rx-', 'LineWidth', 2);
                plot(sscanf(sprintf('%s ', rateR{:}), '%f'), ...
                    nanmedian(loss(2,:,:), 3), ...
                    'b+-', 'LineWidth', 2);
                ylim([0 1]);
                legend({'csma', 'ofdma'}, 'Location', 'northwest');
                xlabel('inject throughput (Kbps)');
                ylabel('loss');
                %%%
%                 pause;
                saveas(gcf, [folderName '/figures/tcp', tcp, ...
                    '-topo', topo, '-mcs', mcs, '-size', psize, '-median.fig']);
                saveas(gcf, [folderName '/figures/tcp', tcp, ...
                    '-topo', topo, '-mcs', mcs, '-size', psize, '-median.jpg']);
                close;
            end
        end
    end
end