topoRange = {'adhoc'};
caRange = {'csma', 'ofdma'};
mcsRange = 0:7;
inRange = 1:6;
rngRange = 0:149;

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
                    A = cell2mat(A(:,2:end));
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
markerRange = {'o', '+'};
colorRange = {'#0072BD', '#D95319', '#EDB120', '#7E2F8E', '#77AC30', ...
    '#4DBEEE', 	'#A2142F', 'k'};
f1 = figure('Position', [800 200 1000 400]);
subplot(1,2,1); hold on; grid on;
for caIndex = 1:length(caRange)
    for mcsIndex = 1:length(mcsRange)
        X = squeeze(thr(1, caIndex, mcsIndex, :, :)); X = X(:);
        Y = squeeze(delay(1, caIndex, mcsIndex, :, :)); Y = Y(:);
        Z = repmat(inRange.', length(rngRange), 1); Z = Z(:);
        scatter3(X, Y, Z, 64, ...
            'Marker', markerRange{caIndex}, ...
            'MarkerEdgeColor', colorRange{mcsIndex}, ...
            'LineWidth', 3);
%         pause;
    end
end
xlabel('per mesh node throughput (Kbps)');
ylabel('per mesh node delay (ms)');
zlabel('per mesh node input (Kbps)');
legend(cellstr([repmat('MCS', size(mcsRange.')), num2str(mcsRange.')]));

subplot(1,2,2); hold on; grid on;
for caIndex = 1:length(caRange)
    for mcsIndex = 1:length(mcsRange)
        X = squeeze(thr(1, caIndex, mcsIndex, :, :)); X = X(:);
        Y = squeeze(loss(1, caIndex, mcsIndex, :, :)); Y = Y(:);
        Z = repmat(inRange.', length(rngRange), 1); Z = Z(:);
        scatter3(X, Y, Z, 64, ...
            'Marker', markerRange{caIndex}, ...
            'MarkerEdgeColor', colorRange{mcsIndex}, ...
            'LineWidth', 3);
%         pause;
    end
end
xlabel('per mesh node throughput (Kbps)');
ylabel('loss rate');
zlabel('per mesh node input (Kbps)');
legend(cellstr([repmat('MCS', size(mcsRange.')), num2str(mcsRange.')]));