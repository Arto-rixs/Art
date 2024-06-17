stopped = false
socket = require("socket")
json = require("json")
IPAddr = "127.0.0.1"
IPPort = 13345
client = nil

-- Глобальная переменная для хранения последней сделки
lastDeal = nil

--- Функция вызывается терминалом QUIK при получении обезличенной сделки
function OnAllTrade(alltrade)
    if alltrade.sec_code == "BRN4" then
        local DealStr = tostring(alltrade.trade_num) .. ";"
            .. tostring(alltrade.datetime.year) .. "-"
            .. tostring(alltrade.datetime.month) .. "-"
            .. tostring(alltrade.datetime.day) .. " "
            .. tostring(alltrade.datetime.hour) .. ":"
            .. tostring(alltrade.datetime.min) .. ":"
            .. tostring(alltrade.datetime.sec) .. "."
            .. tostring(alltrade.datetime.mcs) .. ";"
            .. tostring(alltrade.price) .. ";"
            .. tostring(alltrade.qty) .. ";"
            .. tostring(alltrade.flags)
        
        message("Received trade: " .. DealStr, 1)
        lastDeal = DealStr
    end
end


function OnInit(path)
    server = assert(socket.bind("*", IPPort))
    server:settimeout(0) -- Неблокирующий режим
    message(string.format("Server started. IP: %s; Port: %d\n", IPAddr, IPPort), 1)
    logRequest("Server started.")
end

function OnStop(signal)
    if client then client:close() end
    stopped = true
    logRequest("Server stopped.")
end

function OnClose()
    if client then client:close() end
    stopped = true
    logRequest("Server closed.")
end

function cdelku()
    number_of_trades = getNumberOf("trades")
    message("Общее количество сделок: " .. number_of_trades, 1)
    return "Общее количество сделок: " .. number_of_trades
end

function OnAllTradeCommand()
    -- Возвращение сохраненной информации о последней сделке
    if lastDeal then
        return "Последняя сделка: " .. lastDeal
    else
        return "Нет данных о сделках"
    end
end

function logRequest(message)
    local logFile = io.open("server_log.txt", "a")
    if logFile then
        logFile:write(os.date("%Y-%m-%d %H:%M:%S") .. " - " .. message .. "\n")
        logFile:close()
    else
        message("Failed to open log file", 3)
    end
end

function main()
    while not stopped do
        client = server:accept()
        if client then
            message("Client connected", 1)
            logRequest("Client connected")
            client:settimeout(0)
            while not stopped do
                local line, err = client:receive()
                if line then
                    message("Received command: " .. line, 1)
                    logRequest("Received command: " .. line)
                    local response = nil
                    if line == "cdelku" then
                        response = cdelku()
                    elseif line == "OnAllTrade" then
                        response = OnAllTradeCommand()
                    else
                        response = "Unknown command"
                    end
                    if response then
                        client:send(response .. "\n")
                    end
                elseif err == "closed" then
                    message("Client disconnected", 1)
                    logRequest("Client disconnected")
                    break
                elseif err ~= "timeout" then
                    message("Error receiving data: " .. err, 3)
                    logRequest("Error receiving data: " .. err)
                end
                socket.sleep(1)
            end
            client:close()
        else
            socket.sleep(1)
        end
    end
end
