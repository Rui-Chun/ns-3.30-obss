route = {'aodv', 'olsr', 'potential'};
gate = 2:6;
rnd = 1:20;

totalThr = zeros(length(route), ...
    length(gate), length(rnd));
meanThr = totalThr;

for routeI = 1:length(route)
  for gateI = 1:length(gate)
    for rndI = 1:length(rnd)
      fileName = [route{routeI} '-' ...
          num2str(gate(gateI)) '-' num2str(rnd(rndI)) '.txt'];
      C = dlmread(fileName, '\t', 7, 0);
      totalThr(routeI, gateI, rndI) = sum(mean(C(:,2:end)));
      meanThr(routeI, gateI, rndI) = mean(mean(C(:,2:end)));
    end
  end
end

totalThrA = median(totalThr, 3);
meanThrA = median(meanThr, 3);

figure;
set(gcf, 'Position', [200 200 1080 720]);
bar(gate, meanThrA(:,:).');
set(gca, 'FontSize', 16);
title('one gateway');
xlabel('square grid size');
ylabel('throughput (Mbps)');
legend(route);