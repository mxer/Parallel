close all
clc

figure
grid
hold on

% A = unique(one_node(:, 2));
% for i = 1:numel(A)
%   slices = A(i);
%   times = one_node(one_node(:, 3) == 1024 & one_node(:, 2) == slices & one_node(:, 4) ~= -1, 4);
%   processors = one_node(one_node(:, 3) == 1024 & one_node(:, 2) == slices & one_node(:, 4) ~= -1, 1);
%   plot(processors, times);
% end

slices = two_nodes(two_nodes(:, 3) == 1024, 2); 
times = two_nodes(two_nodes(:, 3) == 1024, 4);
plot(slices, times);


slices = one_node(one_node(:, 3) == 1024 & one_node(:, 1) == 24, 2); 
times = one_node(one_node(:, 3) == 1024 & one_node(:, 1) == 24, 4);
plot(slices, times);

slices = two_nodes(two_nodes(:, 3) == 768, 2); 
times = two_nodes(two_nodes(:, 3) == 768, 4);
plot(slices, times);


slices = one_node(one_node(:, 3) == 768 & one_node(:, 1) == 24, 2); 
times = one_node(one_node(:, 3) == 768 & one_node(:, 1) == 24, 4);
plot(slices, times);

% times = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 64 & one_node(:, 4) ~= -1, 4);
% processors = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 64 & one_node(:, 4) ~= -1, 1);
% plot(processors, times);
% times = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 128 & one_node(:, 4) ~= -1, 4);
% processors = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 128 & one_node(:, 4) ~= -1, 1);
% plot(processors, times);
% times = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 256 & one_node(:, 4) ~= -1, 4);
% processors = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 256 & one_node(:, 4) ~= -1, 1);
% plot(processors, times);
% times = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 512 & one_node(:, 4) ~= -1, 4);
% processors = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 512 & one_node(:, 4) ~= -1, 1);
% plot(processors, times);
% times = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 1024 & one_node(:, 4) ~= -1, 4);
% processors = one_node(one_node(:, 3) == 768 & one_node(:, 2) == 1024 & one_node(:, 4) ~= -1, 1);
% plot(processors, times);