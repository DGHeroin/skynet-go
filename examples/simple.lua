
skynet:OnMessage(function (msg, id)
    print('[lua] OnMessage:', id, #msg)
    skynet:SendMessage(11, 'Hello from Lua!')
    skynet:SendMessage(22, 'Hello again!')

    local reply = skynet:Request(33, 'HeyYa!')
    print('[lua]Request reply:', reply)
end)
skynet:OnRequest(function (data, id)
    print('[lua] OnRequest:', #data, type(data))
    return data
end)
skynet:OnStart(function ()
    print('[lua] OnStart', 4, 5, 6)
end)
skynet:OnStop(function ()
    print('[lua] >>>>OnStop')
end)
