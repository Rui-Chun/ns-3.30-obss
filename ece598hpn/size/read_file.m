route = {'aodv', 'olsr', 'potential'};
grid = 3:5;
gate = 1:2;
rnd = 1:50;

totalThr = zeros(length(route), length(grid), ...
    length(gate), length(rnd));
meanThr = totalThr;
perThr = zeros(length(route), length(gate), ...
    length(rnd), 25);

for routeI = 1:length(route)
  for gridI = 1:length(grid)
    for gateI = 1:length(gate)
      for rndI = 1:length(rnd)
        fileName = [route{routeI} '-' num2str(grid(gridI)) '-' ...
            num2str(gate(gateI)) '-' num2str(rnd(rndI)) '.txt'];
        C = dlmread(fileName, '\t', 7, 0);
        totalThr(routeI, gridI, gateI, rndI) = sum(mean(C(:,2:end)));
        meanThr(routeI, gridI, gateI, rndI) = mean(mean(C(:,2:end)));
        if gridI == 3
          perThr(routeI, gateI, rndI, :) = mean(C(:,2:end));
        end
      end
    end
  end
end

totalThrA = median(totalThr, 4);
meanThrA = median(meanThr, 4);
figure;
set(gcf, 'Position', [200 200 1080 500]);
subplot(2,1,1);
bar(grid, meanThrA(:,:,1).');
set(gca, 'FontSize', 12);
title('one gateway');
xlabel('square grid size');
ylabel('throughput (Mbps)');
legend(route);
subplot(2,1,2);
bar(grid, meanThrA(:,:,2).');
set(gca, 'FontSize', 16);
title('two gateways');
xlabel('square grid size');
ylabel('throughput (Mbps)');
legend(route);

perThrA = squeeze(mean(perThr,3));
figure;
set(gcf, 'Position', [200 200 1080 300]);
for i = 1:3
  subplot(1,3,i);
  imagesc(reshape(perThrA(i,1,:), [5 5]));
end
figure;
set(gcf, 'Position', [200 200 1080 300]);
for i = 1:3
  subplot(1,3,i);
  imagesc(reshape(perThrA(i,2,:), [5 5]));
  title(route{i});
end