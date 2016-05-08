close all
clear all
clc

file_names = dir('results');
formatSpec = '%f, %d, %d\n %f';
sizeA = [1 4];
a = [];
for i = 1:length(file_names)
  if (~file_names(i).isdir)
    fileID = fopen(['results/' file_names(i).name], 'r');
    temp = fscanf(fileID, formatSpec, sizeA);
    if (length(temp) == 3)
      temp = [temp -1];
    end
    a = [a; temp];
    
    fclose(fileID);
  end
end

one_node = a(1:100, :);
two_nodes = a(101:end, :);

% one_node = one_node(one_node(:, 2) >= 32, :);

% a = a';

temp = one_node(one_node(:, 3) == 1024 & one_node(:, 2) == 32, 4);