%% Calculates the mean signal of the 3 aux

function avg_aux = avgSignal(aux_1, aux_2, aux_3)
    size_aux = length(aux_1);
    aux1abs = abs(aux_1);
    aux2abs = abs(aux_2);
    aux3abs = abs(aux_3);
    
    avg_aux = zeros(size_aux,1);
    
    for i=1:size_aux
        avg_aux(i,1) = (aux1abs(i,1)+aux2abs(i,1)+aux3abs(i,1))/3;
    end
end
