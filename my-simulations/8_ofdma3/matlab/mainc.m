topoRange = {'adhoc'};
caRange = {'csma', 'ofdma'};
mcsRange = [0 4 7];
inRange = 2:2:12;
rngRange = 0:49;

thr = zeros(length(topoRange), length(caRange), ...
    length(mcsRange), length(inRange), length(rngRange));
delay = thr;
loss = thr;

for topoIndex = 1:length(topoRange)
    topo = topoRange{topoIndex};
    for caIndex = 1:length(caRange)
        ca = caRange{caIndex};
        for mcsIndex = 1:length(mcsRange)
            mcs = mcsRange(mcsIndex);
            for inIndex = 1:length(inRange)
                inject = inRange(inIndex);
                for rngIndex = 1:length(rngRange)
                    rng = rngRange(rngIndex);
                    filename = ['../', num2str(rng), '/', ...
                        topo, '-', ca, '-mcs', num2str(mcs), ...
                        '-', num2str(inject), 'e5-', num2str(rng), '.out'];
                    if ~isfile(filename)
                        disp('no file found');
                        return;
                    end
                    fileId = fopen(filename);
                    A = textscan(fileId, '%s %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f', 3, ...
                        'Delimiter', '\t', ...
                        'HeaderLines', 67);
                    fclose(fileId);
                    A = cell2mat(A(:,2:end)); A(:,1:5) = [];
                    if size(A,1)~= 3
                        thr(topoIndex, caIndex, mcsIndex, inIndex, rngIndex) = nan;
                        delay(topoIndex, caIndex, mcsIndex, inIndex, rngIndex) = nan;
                        loss(topoIndex, caIndex, mcsIndex, inIndex, rngIndex) = nan;
                    else
                        thr(topoIndex, caIndex, mcsIndex, inIndex, rngIndex) = median(A(1,:));
                        delay(topoIndex, caIndex, mcsIndex, inIndex, rngIndex) = ...
                            A(1,:)*A(2,:).'/sum(A(1,:));
                        loss(topoIndex, caIndex, mcsIndex, inIndex, rngIndex) = median(A(3,:));
                        if loss > 1
                        end
                    end
                end
            end
        end
    end
end

%%
lineRange = {'-', '--'};
markerRange = {'o', '+'};
colorRange = {[0.6350, 0.0780, 0.1840], ...
    [0, 0.4470, 0.7410], ...
    [0.4660, 0.6740, 0.1880], ...
    [0.8500, 0.3250, 0.0980], ...
    [0.3010, 0.7450, 0.9330], ...
    [0.4940, 0.1840, 0.5560], ...
    [0.9290, 0.6940, 0.1250], ...
    [0, 0.5, 0]};
f1 = figure('Position', [800 200 1400 400]);
subplot(1,3,1); hold on; grid on;
for caIndex = 1:length(caRange)
    for mcsIndex = 1:length(mcsRange)
        X = repmat(inRange * 1e2, length(rngRange), 1); X = median(X,1);
        Y = squeeze(thr(1, caIndex, mcsIndex, :, :)); Y = median(Y.',1);
        plot(X, Y, ...
            'LineStyle', lineRange{caIndex}, ...
            'Marker', markerRange{caIndex}, ...
            'MarkerSize', 6, ...
            'Color', colorRange{mcsIndex}, ...
            'LineWidth', 2);
    end
end
xlabel('per mesh node inject (Kbps)');
ylabel('per mesh node throughput (Kbps)');
legend(cellstr([repmat('MCS', size(mcsRange.')), num2str(mcsRange.')]), ...
    'Location', 'northwest');

subplot(1,3,2); hold on; grid on;
for caIndex = 1:length(caRange)
    for mcsIndex = 1:length(mcsRange)
        X = repmat(inRange * 1e2, length(rngRange), 1); X = median(X,1);
        Y = squeeze(delay(1, caIndex, mcsIndex, :, :)); Y = median(Y.',1);
        plot(X, Y, ...
            'LineStyle', lineRange{caIndex}, ...
            'Marker', markerRange{caIndex}, ...
            'MarkerSize', 6, ...
            'Color', colorRange{mcsIndex}, ...
            'LineWidth', 2);
    end
end
xlabel('per mesh node inject (Kbps)');
ylabel('per mesh node delay (ms)');
legend(cellstr([repmat('MCS', size(mcsRange.')), num2str(mcsRange.')]), ...
    'Location', 'northwest');

subplot(1,3,3); hold on; grid on;
for caIndex = 1:length(caRange)
    for mcsIndex = 1:length(mcsRange)
        X = repmat(inRange * 1e2, length(rngRange), 1); X = median(X,1);
        Y = squeeze(loss(1, caIndex, mcsIndex, :, :)); Y = median(Y.',1);
        plot(X, Y, ...
            'LineStyle', lineRange{caIndex}, ...
            'Marker', markerRange{caIndex}, ...
            'MarkerSize', 6, ...
            'Color', colorRange{mcsIndex}, ...
            'LineWidth', 2);
    end
end
xlabel('per mesh node inject (Kbps)');
ylabel('per mesh node loss ');
legend(cellstr([repmat('MCS', size(mcsRange.')), num2str(mcsRange.')]), ...
    'Location', 'northwest');

save_fig_name = fullfile('adhoc.pdf');
% export_fig(save_fig_name, '-painters', '-q101');