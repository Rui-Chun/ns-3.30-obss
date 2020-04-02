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
            A = fscanf(fileId, '(1,%d)\t(2,%d)\t(3,0)\t(4,%d)\t');
            A = reshape(A, 3, []);
            for i = 1:1
                for j = 1:3
                    ru1(mcsIndex, inIndex, j) = ...
                        ru1(mcsIndex, inIndex, j) + A(j,i);
                end
            end
            for i = 2:6
                for j = 1:3
                    ru2(mcsIndex, inIndex, j) = ...
                        ru2(mcsIndex, inIndex, j) + A(j,i);
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
f1 = figure('Position', [800 200 450*length(mcsRange)+50 400]);
for mcsIndex = 1:length(mcsRange)
    subplot(2,length(mcsRange),mcsIndex); hold on; grid on;
    X = repmat(inRange * 1e2, length(rngRange), 1); X = nanmedian(X,1);
    Y = squeeze(ru1(mcsIndex, :, :)) / length(rngRange);
    bar(X, Y);
    xlabel('per mesh node inject (Kbps)');
    ylabel('number of transmissions');
    title(['gateway (mcs' num2str(mcsRange(mcsIndex)) ')']);
    
    subplot(2,length(mcsRange),mcsIndex+length(mcsRange)); hold on; grid on;
    X = repmat(inRange * 1e2, length(rngRange), 1); X = nanmedian(X,1);
    Y = squeeze(ru2(mcsIndex, :, :));
    bar(X, Y);
    xlabel('per mesh node inject (Kbps)');
    ylabel('number of transmissions');
    title(['sum of other nodes (mcs' num2str(mcsRange(mcsIndex)) ')']);
end
subplot(2,length(mcsRange),1);
legend({'1 RU', '2 RUs', '4 RUs'}, ...
    'Location', 'northeast');

%%
save_fig_name = fullfile('ru_num.pdf');
export_fig(save_fig_name, '-painters', '-q101');