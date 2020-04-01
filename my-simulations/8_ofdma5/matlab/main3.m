topo = 'adhoc';
ca = 'ofdma';
mcsRange = [0 4 7];
inRange = 2:1:9;
rngRange = 0:38;

ru1 = zeros(length(mcsRange), length(inRange), 3);
ru2 = ru1;

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
            for i = 1:67
                A = fgetl(fileId);
            end
            for i = 1:1
                A = fscanf(fileId, '(1,%d)\t(2,%d)\t(3,0)\t(4,%d)\t');
                for j = 1:3
                    ru1(mcsIndex, inIndex, j) = ...
                        ru1(mcsIndex, inIndex, j) + A(j);
                end
            end
            for i = 2:5
                A = fscanf(fileId, '(1,%d)\t(2,%d)\t(3,0)\t(4,%d)\t');
                for j = 1:3
                    ru2(mcsIndex, inIndex, j) = ...
                        ru2(mcsIndex, inIndex, j) + A(j);
                end
            end
            fclose(fileId);
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
f1 = figure('Position', [800 200 500 400]);
hold on; grid on;
for caIndex = 1:length(caRange)
    for mcsIndex = 1:length(mcsRange)
        X = repmat(inRange * 1e2, length(rngRange), 1); X = nanmedian(X,1);
        Y = squeeze(ru1(mcsIndex, :, :)); Y = nanmedian(Y.',1);
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
title('OFDMA vs CSMA');

%%
save_fig_name = fullfile('ca_compare.pdf');
export_fig(save_fig_name, '-painters', '-q101');