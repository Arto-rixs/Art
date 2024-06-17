local socket = require("socket")
local json = require("json")

local client = socket.tcp()
local result, err = client:connect("127.0.0.1", 12345)
if not result then
    message("error connect: " .. err, 3) -- Сообщение об ошибке в журнал QUIK
    return
else
    message("connect perfecto", 1) -- Сообщение об успешном подключении в журнал QUIK
end

local data = {
    sec_code = "BR-7.24",
    bid = 150.50,
    ask = 151.00
}

local jsonData = json.encode(data)
message("sending data: " .. jsonData, 1)
local bytesSent, sendError = client:send(jsonData .. "\n")
if sendError then
    message("error otpravku dannux: " .. sendError, 3) -- Сообщение об ошибке отправки данных в журнал QUIK
else
    message("dannux perfecto", 1) -- Сообщение об успешной отправке данных в журнал QUIK
end

-- Ожидание ответа от сервера
local response, receiveError = client:receive()
if receiveError then
    message("error poluxenue dannux: " .. receiveError, 3) -- Сообщение об ошибке получения данных в журнал QUIK
else
    message("otvet ot servera: " .. response, 1) -- Сообщение об успешном получении ответа от сервера в журнал QUIK
end

client:close()
