clc;
clear all;

Fs = 30000;

load('Aux_1_3_dec.mat')
load('frametimestamp.mat')
load('timestamps_signal.mat')


ts = 1/100:1/100:length(Aux1_dec)/100;


avg = avgSignal(Aux1_dec, Aux2_dec, Aux3_dec);

% Por inspeção do vídeo, foi constatado que o rato dormiu entre as frames:
% 12640 -> 18720;
% 39600 -> 50290;
    % 24280 -> 32340; %% parece que era repouso e não sono


% get indexes in witch there was change in the current state of the animal
idx = zeros(4,1);

[idx(1),~] = closestTimestamp(Sourcetimestamp(12640,1),ts_dec);
[idx(2),~] = closestTimestamp(Sourcetimestamp(18720,1),ts_dec);

[idx(3),~] = closestTimestamp(Sourcetimestamp(39600,1),ts_dec);
[idx(4),~] = closestTimestamp(Sourcetimestamp(50290,1),ts_dec);

% view the intervals
plot(ts,avg);
hold on
line([ts(idx(1)) ts(idx(1))], [0 1], 'Color',[1 0 0]);
line([ts(idx(2)) ts(idx(2))], [0 1], 'Color',[1 0 0]);

line([ts(idx(3)) ts(idx(3))], [0 1], 'Color',[1 1 0]);
line([ts(idx(4)) ts(idx(4))], [0 1], 'Color',[1 1 0]);

% change the frequency to 50Hz
aux1_dec = decimate(Aux1_dec,2);
aux2_dec = decimate(Aux2_dec,2);
aux3_dec = decimate(Aux3_dec,2);
avg_dec = decimate(avg, 2); 

idx = round(idx/2);

% define window size
window_size = 256;


windows1 = createRollingWindow(aux1_dec', window_size);
windows2 = createRollingWindow(aux2_dec', window_size);
windows3 = createRollingWindow(aux3_dec', window_size);

% mean features
f11 = mean(windows1');
f12 = mean(windows2');
f13 = mean(windows3');

% standard deviation features
f21 = std(windows1');
f22 = std(windows2');
f23 = std(windows3');

% aux channels correlation features
observations = size(windows1, 1);
 
f31 = zeros(1, observations);
f32 = zeros(1, observations);
f33 = zeros(1, observations);
 
for i=1:observations
    cab = corrcoef(windows1(i,:), windows2(i,:));
    cac = corrcoef(windows1(i,:), windows3(i,:));
    cbc = corrcoef(windows2(i,:), windows3(i,:));
    f31(i) = cab(1,2);
    f32(i) = cac(1,2);
    f33(i) = cbc(1,2);
end

% signal energy features
energy1 = sum(square(abs(fft(windows1', window_size))));
f41 = energy1/window_size;

energy2 = sum(square(abs(fft(windows2', window_size))));
f42 = energy2/window_size;

energy3 = sum(square(abs(fft(windows3', window_size))));
f43 = energy3/window_size;

input = [f11' f12' f13' f21' f22' f23' f31' f32' f33' f41' f42' f43'];

% create response vector
awk1 = ones(1, round(idx(1)));
idx_slp1 = round(idx(2))-round(idx(1));
slp1 = zeros(1, idx_slp1);
idx_awk2 = round(idx(3))-round(idx(2));
awk2 = ones(1, idx_awk2);
idx_slp2 = round(idx(4))-round(idx(3));
slp2 = zeros(1, idx_slp2);

idxs = [awk1 slp1 awk2 slp2];
idx_win = createRollingWindow(idxs, window_size);
output = mode(idx_win')';


train = [input output];
