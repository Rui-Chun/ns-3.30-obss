topoRange = {'adhoc'}
caRange = {'csma', 'ofdma'};
mcsRange = 0:7;
rngRange = 0:59;

thr = zeros(length(topoRange), length(caRange), ...
    length(mcsRange), length(rngRange));
delay = thr;
thr1 = thr;
thr2 = thr;
delay1 = delay;
delay2 = delay;

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
                A = textscan(fileId, '%s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f', 2, ...
                    'Delimiter', '\t', ...
                    'HeaderLines', 67);
                fclose(fileId);
                A = cell2mat(A(:,2:end));
                if size(A,1)~= 2
                    thr(topoIndex, caIndex, mcsIndex, rngIndex) = nan;
                    delay(topoIndex, caIndex, mcsIndex, rngIndex) = nan;
                    thr1(topoIndex, caIndex, mcsIndex, rngIndex) = nan;
                    delay1(topoIndex, caIndex, mcsIndex, rngIndex) = nan;
                    thr2(topoIndex, caIndex, mcsIndex, rngIndex) = nan;
                    delay2(topoIndex, caIndex, mcsIndex, rngIndex) = nan;
                else
                    thr1(topoIndex, caIndex, mcsIndex, rngIndex) = mean(A(1,:));
                    delay1(topoIndex, caIndex, mcsIndex, rngIndex) = ...
                        A(1,:)*A(2,:).'/sum(A(1,:));
                    thr1(topoIndex, caIndex, mcsIndex, rngIndex) = mean(A(1,1:4));
                    delay1(topoIndex, caIndex, mcsIndex, rngIndex) = ...
                        A(1,1:4)*A(2,1:4).'/sum(A(1,1:4));
                    thr2(topoIndex, caIndex, mcsIndex, rngIndex) = mean(A(1,5:end));
                    delay2(topoIndex, caIndex, mcsIndex, rngIndex) = ...
                        A(1,5:8)*A(2,5:8).'/sum(A(1,5:8));
                end
            end
        end
    end
end

figure;
subplot(2,1,1); hold on;
plot(mcsRange, squeeze(nanmedian(thr1(1,1,:,:), 4)).', '-x');
plot(mcsRange, squeeze(nanmedian(thr1(1,2,:,:), 4)).', '-x');
legend('csma', 'ofdma');
xlabel('MCS index');
ylabel('per node throughput (Kbps)');
title('Throughput of 1-hop nodes');
subplot(2,1,2); hold on;
plot(mcsRange, squeeze(nanmedian(thr2(1,1,:,:), 4)).', '-x');
plot(mcsRange, squeeze(nanmedian(thr2(1,2,:,:), 4)).', '-x');
legend('csma', 'ofdma');
xlabel('MCS index');
ylabel('per node throughput (Kbps)');
title('Throughput of 2-hop nodes');

figure;
subplot(2,1,1); hold on;
plot(mcsRange, squeeze(nanmedian(delay1(1,1,:,:), 4)).', '-x');
plot(mcsRange, squeeze(nanmedian(delay1(1,2,:,:), 4)).', '-x');
legend('csma', 'ofdma');
xlabel('MCS index');
ylabel('per node delay (ms)');
title('Delay of 1-hop nodes');
subplot(2,1,2); hold on;
plot(mcsRange, squeeze(nanmedian(delay2(1,1,:,:), 4)).', '-x');
plot(mcsRange, squeeze(nanmedian(delay2(1,2,:,:), 4)).', '-x');
legend('csma', 'ofdma');
xlabel('MCS index');
ylabel('per node delay (ms)');
title('Delay of 2-hop nodes');