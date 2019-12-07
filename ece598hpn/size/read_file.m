route = {'aodv', 'olsr', 'potential'};
grid = 3:5;
gate = 1:2;
rnd = 1:50;

totalThr = zeros(length(route), length(grid), ...
    length(gate), length(rnd));
meanThr = totalThr;

for routeI = 1:length(route)
  for gridI = 1:length(grid)
    for gateI = 1:length(gate)
      for rndI = 1:length(rnd)
        fileName = [route{routeI} '-' num2str(grid(gridI)) '-' ...
            num2str(gate(gateI)) '-' num2str(rnd(rndI)) '.txt'];
        C = dlmread(fileName, '\t', 7, 0);
        totalThr(routeI, gridI, gateI, rndI) = sum(mean(C(:,2:end)));
        meanThr(routeI, gridI, gateI, rndI) = mean(mean(C(:,2:end)));
      end
    end
  end
end

totalThrA = median(totalThr, 4);
meanThrA = median(meanThr, 4);

figure;
subplot(2,1,1);
bar(grid, meanThrA(:,:,1).');
title('two gateway');
xlabel('square grid size');
ylabel('throughput (Mbps)');
legend(route);
subplot(2,1,2);
bar(grid, meanThrA(:,:,2).');
title('one gateways');
xlabel('square grid size');
ylabel('throughput (Mbps)');
legend(route);
