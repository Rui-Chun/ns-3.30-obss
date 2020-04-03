close all; clear; clc;
topoRange = {[2 3], [2 5], [3 5]};
ca = 'ofdma';
mcsRange = [1 5];
inRange = 2:2:10;
rngRange = 0:1;

thr = zeros(length(topoRange), ...
    length(mcsRange), length(inRange), length(rngRange));
delay = thr;
loss = thr;

for topoIndex = 1:length(topoRange)
    topo = topoRange{topoIndex};
    toponame = [];
    nodes = 1;
    skiplines = 1;
    for i = 1:length(topo)
        toponame = [toponame num2str(topo(i))];
        nodes = nodes * topo(i);
        skiplines = skiplines + nodes;
    end
    for mcsIndex = 1:length(mcsRange)
        mcs = mcsRange(mcsIndex);
        for inIndex = 1:length(inRange)
            inject = inRange(inIndex);
            for rngIndex = 1:length(rngRange)
                rng = rngRange(rngIndex);
                filename = ['../', num2str(rng), '/', ...
                    'adhoc', '-', ca, '-mcs', num2str(mcs), ...
                    '-', num2str(inject), 'e5-', num2str(rng), ...
                    '-', toponame, '.out'];
                if ~isfile(filename)
                    disp('no file found');
                    return;
                end
                fileId = fopen(filename);
                A = textscan(fileId, ['%s' repmat(' %f',1,skiplines-1)], 3, ...
                    'Delimiter', '\t', ...
                    'HeaderLines', 67+skiplines);
                fclose(fileId);
                A = cell2mat(A(:,2:end)); A = A(:, end-nodes+2:end);
                if size(A,1)~= 3
                    thr(topoIndex, mcsIndex, inIndex, rngIndex) = nan;
                    delay(topoIndex, mcsIndex, inIndex, rngIndex) = nan;
                    loss(topoIndex, mcsIndex, inIndex, rngIndex) = nan;
                else
                    thr(topoIndex, mcsIndex, inIndex, rngIndex) = median(A(1,:));
                    delay(topoIndex, mcsIndex, inIndex, rngIndex) = ...
                        A(1,:)*A(2,:).'/sum(A(1,:));
                    loss(topoIndex, mcsIndex, inIndex, rngIndex) = median(A(3,:));
                end
            end
        end
    end
end

lineRange = {'-'};
markerRange = {'o'};
colorRange = {[0.6350, 0.0780, 0.1840], ...
    [0, 0.4470, 0.7410], ...
    [0.4660, 0.6740, 0.1880], ...
    [0.8500, 0.3250, 0.0980], ...
    [0.3010, 0.7450, 0.9330], ...
    [0.4940, 0.1840, 0.5560], ...
    [0.9290, 0.6940, 0.1250], ...
    [0, 0.5, 0]};
f1 = figure('Position', [800 200 450*length(mcsRange)+50 400]);
for mcsIndex = 1:length(mcsRange)
    subplot(2,length(mcsRange),mcsIndex);
    box on; hold on; grid on;
    for topoIndex = 1:length(topoRange)
        X = repmat(inRange * 1e2, length(rngRange), 1); X = nanmedian(X,1);
        Y = squeeze(thr(topoIndex, mcsIndex, :, :)); Y = nanmedian(Y.',1);
        plot(X, Y, ...
            'LineStyle', lineRange{1}, ...
            'Marker', markerRange{1}, ...
            'MarkerSize', 6, ...
            'Color', colorRange{topoIndex}, ...
            'LineWidth', 2);
    end
    xlabel('per mesh node inject (Kbps)');
    ylabel('throughput (Kbps)');
    legend(num2str(cell2mat(topoRange.')), ...\
        'Location', 'northwest');
    title(['MCS' num2str(mcsRange(mcsIndex))]);
    
    subplot(2,length(mcsRange),mcsIndex+length(mcsRange));
    box on; hold on; grid on;
    for topoIndex = 1:length(topoRange)
        X = repmat(inRange * 1e2, length(rngRange), 1); X = nanmedian(X,1);
        Y = squeeze(delay(topoIndex, mcsIndex, :, :)); Y = nanmedian(Y.',1);
        plot(X, Y, ...
            'LineStyle', lineRange{1}, ...
            'Marker', markerRange{1}, ...
            'MarkerSize', 6, ...
            'Color', colorRange{topoIndex}, ...
            'LineWidth', 2);
    end
    xlabel('per mesh node inject (Kbps)');
    ylabel('delay (ms)');
end

%%

topoRange = {[3 2], [5 2], [5 3]};
ca = 'ofdma';
mcsRange = [1 5];
inRange = 2:2:10;
rngRange = 0:1;

thr = zeros(length(topoRange), ...
    length(mcsRange), length(inRange), length(rngRange));
delay = thr;
loss = thr;

for topoIndex = 1:length(topoRange)
    topo = topoRange{topoIndex};
    toponame = [];
    nodes = 1;
    skiplines = 1;
    for i = 1:length(topo)
        toponame = [toponame num2str(topo(i))];
        nodes = nodes * topo(i);
        skiplines = skiplines + nodes;
    end
    for mcsIndex = 1:length(mcsRange)
        mcs = mcsRange(mcsIndex);
        for inIndex = 1:length(inRange)
            inject = inRange(inIndex);
            for rngIndex = 1:length(rngRange)
                rng = rngRange(rngIndex);
                filename = ['../', num2str(rng), '/', ...
                    'adhoc', '-', ca, '-mcs', num2str(mcs), ...
                    '-', num2str(inject), 'e5-', num2str(rng), ...
                    '-', toponame, '.out'];
                if ~isfile(filename)
                    disp('no file found');
                    return;
                end
                fileId = fopen(filename);
                A = textscan(fileId, ['%s' repmat(' %f',1,skiplines-1)], 3, ...
                    'Delimiter', '\t', ...
                    'HeaderLines', 67+skiplines);
                fclose(fileId);
                A = cell2mat(A(:,2:end)); A = A(:, end-nodes+2:end);
                if size(A,1)~= 3
                    thr(topoIndex, mcsIndex, inIndex, rngIndex) = nan;
                    delay(topoIndex, mcsIndex, inIndex, rngIndex) = nan;
                    loss(topoIndex, mcsIndex, inIndex, rngIndex) = nan;
                else
                    thr(topoIndex, mcsIndex, inIndex, rngIndex) = median(A(1,:));
                    delay(topoIndex, mcsIndex, inIndex, rngIndex) = ...
                        A(1,:)*A(2,:).'/sum(A(1,:));
                    loss(topoIndex, mcsIndex, inIndex, rngIndex) = median(A(3,:));
                end
            end
        end
    end
end

lineRange = {'--'};
markerRange = {'+'};
colorRange = {[0.6350, 0.0780, 0.1840], ...
    [0, 0.4470, 0.7410], ...
    [0.4660, 0.6740, 0.1880], ...
    [0.8500, 0.3250, 0.0980], ...
    [0.3010, 0.7450, 0.9330], ...
    [0.4940, 0.1840, 0.5560], ...
    [0.9290, 0.6940, 0.1250], ...
    [0, 0.5, 0]};
for mcsIndex = 1:length(mcsRange)
    subplot(2,length(mcsRange),mcsIndex);
    box on; hold on; grid on;
    for topoIndex = 1:length(topoRange)
        X = repmat(inRange * 1e2, length(rngRange), 1); X = nanmedian(X,1);
        Y = squeeze(thr(topoIndex, mcsIndex, :, :)); Y = nanmedian(Y.',1);
        plot(X, Y, ...
            'LineStyle', lineRange{1}, ...
            'Marker', markerRange{1}, ...
            'MarkerSize', 6, ...
            'Color', colorRange{topoIndex}, ...
            'LineWidth', 2);
    end
    xlabel('per mesh node inject (Kbps)');
    ylabel('throughput (Kbps)');
    title(['MCS' num2str(mcsRange(mcsIndex))]);
    
    subplot(2,length(mcsRange),mcsIndex+length(mcsRange));
    box on; hold on; grid on;
    for topoIndex = 1:length(topoRange)
        X = repmat(inRange * 1e2, length(rngRange), 1); X = nanmedian(X,1);
        Y = squeeze(delay(topoIndex, mcsIndex, :, :)); Y = nanmedian(Y.',1);
        plot(X, Y, ...
            'LineStyle', lineRange{1}, ...
            'Marker', markerRange{1}, ...
            'MarkerSize', 6, ...
            'Color', colorRange{topoIndex}, ...
            'LineWidth', 2);
    end
    xlabel('per mesh node inject (Kbps)');
    ylabel('delay (ms)');
end

%%
save_fig_name = fullfile('topo_compare_inverse.pdf');
export_fig(save_fig_name, '-painters', '-q101');