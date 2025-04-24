%% perform filtering with median and Holt-Winters
active_settings = 3;  % 1 - imu, 2 - camera 3 - laser

clock_readings = numbers(:,2) - numbers(1,2);
sensor_readings = numbers(:,3) - numbers(1,3);
current_hyp_t0 = clock_readings - sensor_readings;

switch active_settings
  case 1 % imu
    median_window_size = 1500;

    sensor_framerate = 200;

    alfa = 0.0004;
    beta = 0.0003;

    clamp_time = 120*200;
    early_alfa_start = 0.05;
    early_beta_start = 0.001;

    use_med_filtering = true;
    initial_b = -3.4e-7;
    
  case 2 % camera
    median_window_size = 2500;

    sensor_framerate = 20;

    alfa = 0.003;
    beta = 0.002;

    clamp_time = 500;
    early_alfa_start = 0.1;
    early_beta_start = 0.0;

    use_med_filtering = true;
    initial_b = -3e-7;
    
   case 3 % laser
    median_window_size = 50;

    sensor_framerate = 12.7;

    alfa = 0.002;
    beta = 0.001;

    clamp_time = 500;
    early_alfa_start = 0.02;
    early_beta_start = 0.01;

    use_med_filtering = false;
    initial_b = 0;
end

%%%%

n = size(current_hyp_t0, 1);
s = zeros(n, 1);
b = s;

if use_med_filtering
    med_filtered = medfilt(median_window_size, current_hyp_t0);
else
    med_filtered = current_hyp_t0; % skip filtering
end

% the median filter needs ~500 samples to stabilize itself, we clamp to it
% until this time passes

b(1)= initial_b;
s(1) = med_filtered(1);

for i=2:n
    if i < clamp_time
        progress=1-exp(0.5*(1-1/(1-i/clamp_time)));
        early_alfa= progress*alfa+(1-progress)*early_alfa_start;
        early_beta= progress*beta+(1-progress)*early_beta_start;
      s(i)=early_alfa*med_filtered(i)+(1-early_alfa)*(s(i-1)+b(i-1));
      b(i)=early_beta*(s(i)-s(i-1))+(1-early_beta)*b(i-1);
    else
      s(i)=alfa*med_filtered(i)+(1-alfa)*(s(i-1)+b(i-1));
      b(i)=beta*(s(i)-s(i-1))+(1-beta)*b(i-1);
    end
end

x = (1:n)/sensor_framerate;
figure(1);
hold off;
plot(x, current_hyp_t0);
hold on; 
if use_med_filtering
    plot(x, med_filtered);
end
plot(x, s);
if use_med_filtering
    legend('original t0 (t_{sys} - t_{sensor})', 'median filtered (input)', 'Holt-Winters filtered')
else
    legend('original t0 (t_{sys} - t_{sensor})', 'Holt-Winters filtered')
end
xlabel('t [seconds]')
figure(2); 
hold off;
plot(x(clamp_time:end), b(clamp_time:end));
title('estimated drift speed')
xlabel('t [seconds]')
ylabel('drift [seconds/sample]')
figure(3);
hold off;
plot(x(clamp_time:end), current_hyp_t0(clamp_time:end)- s(clamp_time:end))
title('error (current t0 hypothesis - smoothed t0 hypothesis)')
figure(4);
hold off
plot(x(2:end), diff(clock_readings))
hold on
plot(x(2:end), diff(s+sensor_readings))
xlabel('t [seconds]')
title('inter-sample time delta')
legend('system clock', 'final filtered timestamp')
