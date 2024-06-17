local socket = require("socket")

local client = socket.tcp()
local result, err = client:connect("127.0.0.1", 12345)

if not result then
    message("error: " .. err, 3) -- Сообщение об ошибке в журнал QUIK
    return
else
    message("connect perfecto", 1) -- Сообщение об успешном подключении в журнал QUIK
end
