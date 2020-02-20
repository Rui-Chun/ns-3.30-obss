topoRange = {'apsta'}
caRange = {'csma', 'ofdma'};
mcsRange = 0:7;
injectRange = (2:9)*1e3;
rngRange = 0:14;

thr = zeros(length(topoRange), length(caRange), ...
    length(mcsRange), length(rngRange));
delay = thr;

for topoIndex = 1:length(topoRange)
    topo = topoRange{topoIndex};
    for caIndex = 1:length(caRange)
        ca = caRange{caIndex};
        for mcsIndex = 1:length(mcsRange)
            mcs = mcsRange(mcsIndex);
            for rngIndex = 1:length(rngRange)
                rng = rngRange(rngIndex);
                filename = ['../', num2str(rng), '/', ...
                    topo, '-', ca, '-mcs', num2str(mcs), ...
                    '-', num2str(rng), '.out'];
                if ~isfile(filename)
                    disp('no file found');
                    return;
                end
                fileId = fopen(filename);
                A = textscan(fileId, '%s %f %f %f %f', 2, ...
                    'Delimiter', '\t', ...
                    'HeaderLines', 67);
                fclose(fileId);
                A = cell2mat(A(:,2:end));
                if size(A,1)~= 2
                    thr(topoIndex, caIndex, mcsIndex, rngIndex) = nan;
                    delay(topoIndex, caIndex, mcsIndex, rngIndex) = nan;
                else
                    thr(topoIndex, caIndex, mcsIndex, rngIndex) = mean(A(1,:));
                    delay(topoIndex, caIndex, mcsIndex, rngIndex) = ...
                        A(1,:)*A(2,:).'/sum(A(1,:));
                end
            end
        end
    end
end

figure; hold on;
plot(mcsRange, squeeze(nanmean(thr(1,1,:,:), 4)).', '-x');
plot(mcsRange, squeeze(nanmean(thr(1,2,:,:), 4)).', '-x');
plot(mcsRange, injectRange, '--x');
legend('csma', 'ofdma', 'inject');
xlabel('MCS index');
ylabel('per node throughput (Kbps)');
title('AP with 4 STA average throughput');
saveas(gca, 'apsta-thr.jpg');

figure; hold on;
plot(mcsRange, squeeze(nanmean(delay(1,1,:,:), 4)).', '-x');
plot(mcsRange, squeeze(nanmean(delay(1,2,:,:), 4)).', '-x');
legend('csma', 'ofdma');
xlabel('MCS index');
ylabel('per node delay (ms)');
title('AP with 4 STA average delay');
saveas(gca, 'apsta-delay.jpg');