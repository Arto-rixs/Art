local sec_code = "BRN4"
local class_code = "SPBFUT"

-- Получение данных о котировке
local function getQuoteData(sec_code, class_code)
    local bid = tonumber(getParamEx(class_code, sec_code, "BID").param_value)
    local ask = tonumber(getParamEx(class_code, sec_code, "OFFER").param_value)
    return bid, ask
end

-- Подключение необходимых модулей
local socket = require("socket")
local json = require("json")

-- Функция для отправки данных на сервер
local function sendData()
    local client = socket.tcp()
    local result, err = client:connect("127.0.0.1", 12345)

    if not result then
        message("error connect: " .. err, 3)
        return
    else
        message("connect perfecto", 1)
    end

    local bid, ask = getQuoteData(sec_code, class_code)
    local data = {
        sec_code = sec_code,
        bid = bid,
        ask = ask
    }

    local jsonData = json.encode(data)
    message("sending data: " .. jsonData, 1)
    local bytesSent, sendError = client:send(jsonData .. "\n")
    if sendError then
        message("error otpravku dannux: " .. sendError, 3)
    else
        message("dannux perfecto", 1)
    end

    local response, receiveError = client:receive()
    if receiveError then
        message("error poluxenue dannux: " .. receiveError, 3)
    else
        message("otvet ot servera: " .. response, 1)
    end

    client:close()
end

-- Вызов функции sendData() один раз
sendData()
()
    
--]]

--[[ с проверкой коннекта!
local sec_code = "BRN4"
local class_code = "SPBFUT"

-- Получение данных о котировке
local function getQuoteData(sec_code, class_code)
    local bid = tonumber(getParamEx(class_code, sec_code, "BID").param_value)
    local ask = tonumber(getParamEx(class_code, sec_code, "OFFER").param_value)
    return bid, ask
end

-- Подключение необходимых модулей
local socket = require("socket")
local json = require("json")

-- Функция для отправки данных на сервер
function sendData()
    local client = socket.tcp()
    local result, err = client:connect("127.0.0.1", 12345)

    if not result then
        message("error connect: " .. err, 3)
        return
    else
        message("connect perfecto", 1)
    end

    local bid, ask = getQuoteData(sec_code, class_code)
    local data = {
        sec_code = sec_code,
        bid = bid,
        ask = ask
    }

    local jsonData = json.encode(data)
    message("sending data: " .. jsonData, 1)
    local bytesSent, sendError = client:send(jsonData .. "\n")
    if sendError then
        message("error otpravku dannux: " .. sendError, 3)
    else
        message("dannux perfecto", 1)
    end

    local response, receiveError = client:receive()
    if receiveError then
        message("error poluxenue dannux: " .. receiveError, 3)
    else
        message("otvet ot servera: " .. response, 1)
    end

    client:close()
end

-- Бесконечный цикл для отправки данных
while true do
    sendData()
    -- Задержка между отправками данных (например, 5 секунд)
    socket.sleep(5)
end
--]]