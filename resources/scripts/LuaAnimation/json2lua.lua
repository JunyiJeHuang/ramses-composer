require("lfs")
local json = require("json")
UtilModule = require('UtilModule')

function string.starts(String, Start)
    return string.sub(String, 1, string.len(Start)) == Start
end

function string.ends(String, End)
    return End == '' or string.sub(String, -string.len(End)) == End
end

local ROOT_DIR = '../'

local function findProjectJsonFile()
    local fname = ''
    for entry in lfs.dir(ROOT_DIR) do
        if string.ends(entry, '.json') then
            fname = entry
            break
        end
    end
    return fname
end

local pjf = arg[1] or findProjectJsonFile()

local function readProjectJson()
    local f = io.open(ROOT_DIR..pjf, 'r');
    assert(f);
    local data = f:read('*a');
    f:close();
    return data
end

local function writeLuaScriptToFile(tableData)
    local file = io.open('./LuaJsonDataModule.lua', 'w');
    assert(file);

    local luaScriptString = [[
-- Automatic generation !!!
local LuaJsonDataModule = {}
local data = %s
function LuaJsonDataModule.getData()
    return data
end
return LuaJsonDataModule
    ]]

    file:write(string.format(luaScriptString, UtilModule.serialize(tableData)));
    file:close();
end


local tableData = json.decode(readProjectJson())

writeLuaScriptToFile(tableData)
