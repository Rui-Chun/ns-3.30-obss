route = {'aodv', 'olsr', 'potential'};
grid = 3:5;
gate = 1:2;
rnd = 1:50;

totalThr = zeros(length(route), length(grid), ...
    length(gate), length(rnd));

for routeI = 1:length(route)
  for gridI = 1:length(grid)
    for gateI = 1:length(gate)
      for rndI = 1:length(rnd)
        fileName = [route{routeI} '-' num2str(grid(gridI)) '-' ...
            num2str(gate(gateI)) '-' num2str(rnd(rndI)) '.txt'];
        C = dlmread(fileName, '\t', 7, 0);
        totalThr(routeI, gridI, gateI, rndI) = sum(sum(C(:,2:end)));
      end
    end
  end
end

totalThrA = median(totalThr, 4);

figure;
