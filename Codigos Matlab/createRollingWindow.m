%% Create a set of rolling window with a specified size for a data line vector

function output = createRollingWindow(seq, window_size)
    output = [seq(1:window_size)];
    
    num_windows = floor((length(seq)/(window_size/2)) - 1);
    
    for i=1:num_windows-1
        output = [output; seq((i*(window_size/2) + 1):((i+2)*(window_size/2)))];
    end
end
