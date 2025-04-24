%% loads the dataset in stamps_parsed.log

%mex swallow_csv.cpp
[numbers, text] = swallow_csv('stamps_parsed.log', '"', ' ');
text = text(1, [2*(1:10)-1]);
numbers = numbers(:, [2*(1:10)]);

% figure(1)
% plot(numbers(:,6))
% hold on
% plot(numbers(:, 10))
% title('Smoothed and current t0 hypothesis')
% 
% figure(2)
% plot(numbers(:, 9))
% title('Error (system-sensor)')
