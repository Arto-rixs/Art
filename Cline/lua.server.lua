local socket = require("socket")
local json = require("json")
local IPAddr = "127.0.0.1"
local IPPort = 13345

local clients = {}
local stopped = false

-- Глобальная переменная для хранения последних сделок
local lastTrades = {}

-- Функция для логирования сообщений
local function logRequest(message)
    local logFile = io.open("server_log.txt", "a")
    if logFile then
        logFile:write(os.date("%Y-%m-%d %H:%M:%S") .. " - " .. message .. "\n")
        logFile:close()
    else
        message("Failed to open log file", 3)
    end
end

-- Функция для рассылки сообщений всем подключенным клиентам
local function broadcast(message, isHeartbeat)
    for i = #clients, 1, -1 do
        local client = clients[i]
        local success, err = client:send(message)
        if not success then
            table.remove(clients, i)
            logRequest("Removed client due to send error: " .. tostring(err))
        end
    end
end

-- Функция вызывается терминалом QUIK при получении обезличенной сделки
function OnAllTrade(alltrade)
    if alltrade.sec_code == "BRN4" then
        local DealStr = {
            trade_num = alltrade.trade_num,
            datetime = {
                year = alltrade.datetime.year,
                month = alltrade.datetime.month,
                day = alltrade.datetime.day,
                hour = alltrade.datetime.hour,
                min = alltrade.datetime.min,
                sec = alltrade.datetime.sec,
                mcs = alltrade.datetime.mcs
            },
            price = alltrade.price,
            qty = alltrade.qty,
            flags = alltrade.flags
        }

        local jsonData = json.encode(DealStr)
        message("Sending trade data: " .. jsonData, 1)
        broadcast("Received trade: " .. jsonData .. "\n")

        -- Сохраняем последнюю сделку
        table.insert(lastTrades, DealStr)
        if #lastTrades > 100 then -- Хранить только последние 100 сделок
            table.remove(lastTrades, 1)
        end

        -- Отправка данных всем подключенным клиентам
        broadcast("Received trade: " .. jsonData .. "\n")
    end
end

function OnInit(path)
    server = assert(socket.bind("*", IPPort))
    server:settimeout(0) -- Неблокирующий режим
    message(string.format("Server started. IP: %s; Port: %d\n", IPAddr, IPPort), 1)
    logRequest("Server started.")
end

function OnStop(signal)
    for i = #clients, 1, -1 do
        local client = clients[i]
        client:close()
        table.remove(clients, i)
    end
    stopped = true
    logRequest("Server stopped.")
end

function OnClose()
    for i = #clients, 1, -1 do
        local client = clients[i]
        client:close()
        table.remove(clients, i)
    end
    stopped = true
    logRequest("Server closed.")
end

function cdelku()
    local number_of_trades = getNumberOf("trades")
    message("Общее количество сделок: " .. number_of_trades, 1)
    return json.encode({ total_trades = number_of_trades })
end

function OnAllTradeCommand()
    return json.encode(lastTrades)
end

function main()
    while not stopped do
        local client = server:accept()
        if client then
            message("Client connected", 1)
            logRequest("Client connected")
            client:settimeout(0)
            table.insert(clients, client)
        end
        
        for i = #clients, 1, -1 do
            local client = clients[i]
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
                    response = json.encode({ error = "Unknown command" })
                end
                if response then
                    client:send(response .. "\n")
                end
            elseif err == "closed" then
                message("Client disconnected", 1)
                logRequest("Client disconnected")
                table.remove(clients, i)
            elseif err ~= "timeout" then
                message("Error receiving data: " .. err, 3)
                logRequest("Error receiving data: " .. err)
                table.remove(clients, i)
            end
        end
        
        
        socket.sleep(1)
    end
end
 