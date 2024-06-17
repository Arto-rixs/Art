stopped = false -- Остановка файла
socket = require("socket") -- Указатель для работы с sockets
json = require( "json" ) -- Указатель для работы с json
IPAddr = "127.0.0.1" --IP Адрес
IPPort = 13345		 --IP Port	 
client = nil
   
-- Функция вызывается перед вызовом main
function OnInit(path)
  -- create a TCP socket and bind it to the local host, at port IPPort
  server = assert(socket.bind("*", IPPort))
  message(string.format("Server started. IP: %s; Port: %d\n", IPAddr, IPPort), 1);
end;

-- Функция вызывается перед остановкой скрипта
function OnStop(signal)
  if client then client:close() end
  stopped = true; -- Остановили исполнение кода 
end;

-- Функция вызывается перед закрытием квика
function OnClose()
  if client then client:close() end
  stopped = true; -- закрыли квик, надо остановить исполнение кода
end;

--Этой функцией заменен парсер. Саму ее нужно применять с осторожностью, т.к. это может нарушить безопасность системы.
--Она позволяет выполнять любой луа-код, поступивший со стороны клиента
function evalString (str)
  return assert(loadstring( "return "  .. str))()
end

 
-- Основная функция выполнения скрипта
function main()
-- wait for a connection from any client
  client = server:accept()
  sleep(1)
  while not stopped do
    local line, err = client:receive()
-- if there was no error, send it back to the client
    local result = evalString(line)
    if result == nil  then result = "{}" end
    if type(result) == "table"  then result = json.encode(result) end
    if type(result) == "boolean"  then result = (result and 1 or 0) end
--    if not err then message(string.format("Message:%s Result: %s\n", line, result), 1) end
    if not err then client:send(result .. "\n") end
  end
end;
