close all; clear; clc;
topoRange = {[4], [8]};
% topoRange = {[2 2], [2 3], [2 4], [2 5], ...
%     [3 2], [3 3], [3 4], [3 5], ...
%     [4 2], [4 3], [4 4], ...
%     [5 2], [5 3], ...
%     [2 2 2], [2 2 3], [2 3 2], [3 2 2]};

for topoIndex = 1:length(topoRange)
    topo = [1 topoRange{topoIndex}];
    locationFile = 'locationFile';
    routingFile = 'routingFile';
    
    %% location
    theta = cell(1,length(topo));
    r = zeros(1,length(topo));
    x = cell(1,length(topo));
    y = cell(1,length(topo));
    theta{1} = 0;
    x{1} = 0;
    y{1} = 0;
    n = 1;
    for i = 2:length(topo)
        locationFile = [locationFile num2str(topo(i))];
        routingFile = [routingFile num2str(topo(i))];
        
        n = n * topo(i);
        temp = 2*pi/n;
        theta{i} = temp/2:temp:2*pi;
        if i == 2
            r(i) = 40;
        else
            temp1 = theta{i-1}(1)-theta{i}(1);
            r(i) = r(i-1)*cos(temp1) + sqrt(40^2-(r(i-1)*sin(temp1))^2);
        end
        x{i} = r(i) * cos(theta{i});
        y{i} = r(i) * sin(theta{i});
        %     disp(norm([x{i}(1)-x{i-1}(1); y{i}(1)-y{i-1}(1)]));
    end
    
    locationFile = [locationFile '.txt'];
    routingFile = [routingFile '.txt'];
    
    % figure;
    % box on; hold on; grid on;
    % for i = 1:length(topo)
    %     scatter(x{i}, y{i});
    % end
    % axis equal;
    
    %% locationFile
    fileId = fopen(locationFile, 'w');
    for i = 1:length(topo)
        for j = 1:length(theta{i})
            if i == length(topo)
                fprintf(fileId, '%.2f\t%.2f\t%.2f\t%.2f\n', ...
                    x{i}(j), y{i}(j), 0, 1);
            else
                fprintf(fileId, '%.2f\t%.2f\t%.2f\t%.2f\n', ...
                    x{i}(j), y{i}(j), 0, 0);
            end
        end
    end
    fclose(fileId);
    
    %% routingFile
    A = 0;
    B = [];
    n = 1;
    for i = 2:length(topo)
        n = n * topo(i);
        A = repmat(A, [topo(i), 1]);
        A = sortrows(A, 1:size(A,2), 'ascend');
        A = [A, (A(end,end)+1:A(end,end)+n).'];
        for j = 1:size(A,1)
            for k = 1:size(A,2)-1
                B = [B; ...
                    A(j,k), A(j,end), A(j,k+1)];
            end
            B = [B; ...
                A(j,end), 0, A(j,end-1)];
        end
    end
    B = sortrows(B, 1:size(B,2), 'ascend');
    fileId = fopen(routingFile, 'w');
    for i = 1:size(B,1)
        fprintf(fileId, '%d\t%d\t%d\n', B(i,:));
    end
    fclose(fileId);
end