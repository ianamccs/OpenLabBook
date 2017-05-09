%% Return the closest index and value to a reference value in a vector

function [idx, closest] = closestTimestamp(val, vector)

    tmp = abs(vector-val);
    [idx idx] = min(tmp); %index of closest value
    closest = vector(idx); %closest value
    
end
