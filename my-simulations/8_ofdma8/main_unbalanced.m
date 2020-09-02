fclose('all');
clear; clc;

tcpR = {'0'};
topoR = {'42'};
topo2R = {'a', 'b1', 'b2', 'c', 'd', 'e1', 'e2'};
mcsR = {'3', '4', '5', '6'};
psizeR = {'4e3'};
rateR = {'5e6', '10e6', '15e6', '20e6', '25e6', '30e6'};
kR = 1:50;

folderName = 'unbalanced';

for tcpI = 1:length(tcpR)
    tcp = tcpR{tcpI};
    for topoI = 1:length(topoR)
        topo = topoR{topoI};
        totalN = 1;
        for i = length(topo):-1:1
            totalN = totalN * str2num(topo(i)) + 1;
        end
        scanForm = ['%s', repmat(' %f', [1, totalN-1])];
        for topo2I = 1:length(topo2R)
            topo2 = topo2R{topo2I};
            locationFile = [folderName, '/location/locationFile', topo, topo2, '.txt'];
            fileId = fopen(locationFile);
            A = textscan(fileId, '%f %f %f %f', ...
                'Delimiter', '\t');
            A = cell2mat(A);
            fclose(fileId);
            scanNode = find(A(:,4)==1);
            for mcsI = 1:length(mcsR)
                mcs = mcsR{mcsI};
                for sizeI = 1:length(psizeR)
                    psize = psizeR{sizeI};
                    thr = nan(2, length(rateR), length(kR));
                    delay = nan(2, length(rateR), length(kR));
                    loss = nan(2, length(rateR), length(kR));
                    thrC = cell(2, length(rateR), length(kR));
                    delayC = cell(2, length(rateR), length(kR));
                    lossC = cell(2, length(rateR), length(kR));                    
                    for rateI = 1:length(rateR)
                        rate = rateR{rateI};
                        for k = kR
                            % csma
                            fileName = [folderName, ...
                                '/topo', topo, topo2, '-mcs', mcs, '-size', psize, ...
                                '/csma-topo', topo, '-mcs', mcs, ...
                                '-rate', rate, '-size', psize, '-tcp', tcp, ...
                                '-', num2str(k), '.txt'];
                            fileId = fopen(fileName);
                            A = textscan(fileId, scanForm, 3, ...
                                'HeaderLines', 37, 'Delimiter', '\t');
                            fclose(fileId);
                            try
                                A = cell2mat(A(scanNode));
                                thr(1, rateI, k) = mean(A(1,:));
                                delay(1, rateI, k) = A(2,:) * A(1,:).' / sum(A(1,:));
                                loss(1, rateI, k) = A(3,:) * A(1,:).' / sum(A(1,:));
                                thrC{1, rateI, k} = A(1,:);
                                delayC{1, rateI, k} = A(2,:);
                                lossC{1, rateI, k} = A(3,:);
                            catch
                            end
                            % ofdma
                            fileName = [folderName, ...
                                '/topo', topo, topo2, '-mcs', mcs, '-size', psize, ...
                                '/ofdma-topo', topo, '-mcs', mcs, ...
                                '-rate', rate, '-size', psize, '-tcp', tcp, ...
                                '-', num2str(k), '.txt'];
                            fileId = fopen(fileName);
                            A = textscan(fileId, scanForm, 3, ...
                                'HeaderLines', 37+totalN, 'Delimiter', '\t');
                            fclose(fileId);
                            try
                                A = cell2mat(A(scanNode));
                                thr(2, rateI, k) = mean(A(1,:));
                                delay(2, rateI, k) = A(2,:) * A(1,:).' / sum(A(1,:));
                                loss(2, rateI, k) = A(3,:) * A(1,:).' / sum(A(1,:));
                                thrC{2, rateI, k} = A(1,:);
                                delayC{2, rateI, k} = A(2,:);
                                lossC{2, rateI, k} = A(3,:);
                            catch
                            end
                        end
                    end
                    
                    %% figure
                    figure('Position', [0 0 1600 1400]);
                    %%% scatter
                    subplot(3,2,1);
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
                    title(['tcp', tcp, '-topo', topo, topo2, '-mcs', mcs, '-size', psize]);
                    subplot(3,2,2);
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
                    subplot(3,3,7);
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
                    title(['tcp', tcp, '-topo', topo, topo2, '-mcs', mcs, '-size', psize]);
                    subplot(3,3,8);
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
                    subplot(3,3,9);
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
                    %%% scatter
                    subplot(3,2,3);
                    box on; hold on; grid on;
                    scatter(reshape(cell2mat(reshape(thrC(1,:,:), 1, [])), 1, []), ...
                        reshape(cell2mat(reshape(delayC(1,:,:), 1, [])), 1, []), ...
                        'rx', 'LineWidth', 2);
                    scatter(reshape(cell2mat(reshape(thrC(2,:,:), 1, [])), 1, []), ...
                        reshape(cell2mat(reshape(delayC(2,:,:), 1, [])), 1, []), ...
                        'b+', 'LineWidth', 2);
                    legend({'csma', 'ofdma'}, 'Location', 'northwest');
                    xlabel('throughput (Kbps)');
                    ylabel('delay (ms)');
                    title(['tcp', tcp, '-topo', topo, topo2, '-mcs', mcs, '-size', psize]);
                    subplot(3,2,4);
                    box on; hold on; grid on;
                    scatter(reshape(cell2mat(reshape(thrC(1,:,:), 1, [])), 1, []), ...
                        reshape(cell2mat(reshape(lossC(1,:,:), 1, [])), 1, []), ...
                        'rx', 'LineWidth', 2);
                    scatter(reshape(cell2mat(reshape(thrC(2,:,:), 1, [])), 1, []), ...
                        reshape(cell2mat(reshape(lossC(2,:,:), 1, [])), 1, []), ...
                        'b+', 'LineWidth', 2);
                    legend({'csma', 'ofdma'}, 'Location', 'northwest');
                    ylim([0 1]);
                    xlabel('throughput (Kbps)');
                    ylabel('loss');
                    %%%
%                     pause;
%                     saveas(gcf, [folderName '/figures/tcp', tcp, ...
%                         '-topo', topo, topo2, '-mcs', mcs, '-size', psize, '.fig']);
%                     saveas(gcf, [folderName '/figures/tcp', tcp, ...
%                         '-topo', topo, topo2, '-mcs', mcs, '-size', psize, '.jpg']);
                    close;
                end
            end
        end
    end
end