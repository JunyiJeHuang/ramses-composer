-------------------------------------------------------------------------------
-- Description: Utility function
-------------------------------------------------------------------------------

local UtilModule = {}

-- Splitting strings into arrays
function UtilModule.split(str, reps)
    local res = {}
    string.gsub(str, '[^' .. reps .. ']+', function(w)
        table.insert(res, w)
    end)
    return res
end

-- Array to String
function UtilModule.join(tab, reps)
    if reps == nil then
        reps = ''
    end
    return table.concat(tab, reps)
end

-- Delete the last element of the array and return the value of the last element (the original array has been modified)
function UtilModule.pop(arr)
    return table.remove(arr, #arr)
end

-- Array truncation, returns a new array
function UtilModule.slice(arr, first, last, step)
    local sliced = {}
    for i = first or 1, last or #arr, step or 1 do
        sliced[#sliced + 1] = arr[i]
    end
    return sliced
end

-- Append data backwards to an array
function UtilModule.push(arr, item)
    arr[#arr + 1] = item
    return arr
end

-- Array contains
function UtilModule.includes(arr, item)
    for i = 1, #arr do
        if arr[i] == item then
            return true
        end
    end
    return false
end

function UtilModule.serialize(obj)
    local lua = ''
    local t = type(obj)
    if t == 'number' then
        lua = lua .. obj
    elseif t == 'boolean' then
        lua = lua .. tostring(obj)
    elseif t == 'string' then
        lua = lua .. string.format('%q', obj)
    elseif t == 'table' then
        lua = lua .. '{\n'
        for k, v in pairs(obj) do
            lua = lua .. '[' .. UtilModule.serialize(k) .. ']=' .. UtilModule.serialize(v) .. ',\n'
        end
        local metatable = getmetatable(obj)
        if metatable ~= nil and type(metatable.__index) == 'table' then
            for k, v in pairs(metatable.__index) do
                lua = lua .. '[' .. UtilModule.serialize(k) .. ']=' .. UtilModule.serialize(v) .. ',\n'
            end
        end
        lua = lua .. '}'
    elseif t == 'nil' then
        return nil
    else
        error('Can not serialize a ' .. t .. 'type.')
    end
    return lua
end

-- Start of string
function UtilModule.startWith(str, substr)
    if str == nil or substr == nil then
        return nil, 'The string or the sub-stirng parameter is nil'
    end
    if string.find(str, substr) ~= 1 then
        return false
    else
        return true
    end
end

-- End of string
function UtilModule.endWith(str, substr)
    if str == nil or substr == nil then
        return nil, 'The string or the sub-string parameter is nil'
    end
    local str_tmp = string.reverse(str)
    local substr_tmp = string.reverse(substr)
    if string.find(str_tmp, substr_tmp) ~= 1 then
        return false
    else
        return true
    end
end

return UtilModule
