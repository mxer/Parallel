evenprocess = (rem(i,2) == 0);
evenphase = 1;
for step = 0:N-1
    if (evenprocess)
        if (evenphase)
        // even phase even processor
            if (i != last_processor)
                send(a, i+1);
                receive(x, i+1);
                if x < a
                    a = x;
                end
            end
        else
        // odd phase even processor
            if (i != first_processor)
                send(a, i-1);
                receive(x, i-1);
                if x > a
                    x = a;
                end
            end
        end
    else
        if (evenphase)
        // even phase odd processor
            if (i != first_processor)
                receive(a, i-1);
                send(x, i-1);
                if x < a
                    x = a;
                end
            end
        else
        // odd phase odd processor
            if (i != last_processor)
                receive(a, i-1);
                send(x, i-1);
                if x > a
                    x = a;
                end
            end
        end
    end
    evenphase = ~evenphase;
end