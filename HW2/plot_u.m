clear all
close all

fileID = fopen('u_result_4.txt','r');
% u = zeros(1002, 1);
u = fscanf(fileID,'%f');
fclose(fileID);

x = 0:(1 / (length(u) - 1)):1;


figure
plot(x, u)
title('Calculated u')
grid


% u = 3 * sin(6 * pi * x);  % 3
u = 10 * sin(100 * pi * x);  % 4
figure
plot(x, u)
title('Original u')
grid