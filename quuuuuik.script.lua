--Вывод сообщения с количеством сделок
function ds()
    number_of_trades = getNumberOf("trades")
    message("Общее количество сделок: " .. number_of_trades)
    
end

function main()
    ds()
end