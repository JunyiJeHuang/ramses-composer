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

-- Define a function that takes two colors in RGBA format and a blending factor
function UtilModule.blendColors(color1, color2, HUD)
    -- Extract the individual color components from each color
    local r1, g1, b1, a1 = color1[1], color1[2], color1[3], color1[4]
    local r2, g2, b2, a2 = color2[1], color2[2], color2[3], color2[4]

    -- Calculate the blended color components using the provided blending factor
    local r = r1 * (1 - HUD) + r2 * HUD
    local g = g1 * (1 - HUD) + g2 * HUD
    local b = b1 * (1 - HUD) + b2 * HUD
    local a = a1 * (1 - HUD) + a2 * HUD

    if a > 1 then
        a = a / 255
    end

    if r > 1 then
        r = r / 255
    end

    if g > 1 then
        g = g / 255
    end

    if b > 1 then
        b = b / 255
    end

    -- Return the blended color as a new RGBA color
    return { r, g, b, a }
end

-- Define a function that takes two colors in RGBA format and a blending factor
function UtilModule.blendColors2(color1, color2, HUD)
    -- Extract the individual color components from each color
    local r1, g1, b1, a1 = color1[1], color1[2], color1[3], color1[4]
    local r2, g2, b2, a2 = color2[1], color2[2], color2[3], color2[4]

    local a = (a1 * HUD) + a2 * (1 - HUD)
    local r = r1 * a + r2 * (1 - a)
    local g = g1 * a + g2 * (1 - a)
    local b = b1 * a + b2 * (1 - a)

    -- Calculate the blended color components using the provided blending factor
    -- local r = r1 * (1 - HUD) + r2 * HUD
    -- local g = g1 * (1 - HUD) + g2 * HUD
    -- local b = b1 * (1 - HUD) + b2 * HUD
    -- local a = a1 * (1 - HUD) + a2 * HUD

    -- Return the blended color as a new RGBA color
    if a > 1 then
        a = a / 255
    end

    if r > 1 then
        r = r / 255
    end

    if g > 1 then
        g = g / 255
    end

    if b > 1 then
        b = b / 255
    end

    return { r, g, b, a }
end

-- function UtilModule.blendColors3(color1, color2, HUD)
--     local r1, g1, b1, a1 = color1[1], color1[2], color1[3], color1[4]
--     local r2, g2, b2, a2 = color2[1], color2[2], color2[3], color2[4]

--     a1 = a1 / 256.0
--     a2 = a2 / 256.0
--     local a = 1 - (1 - a1) * (1 - a2)

--     local r = (a1 * r1 + (1 - a1) * a2 * r2) / a
--     local g = (a1 * g1 + (1 - a1) * a2 * g2) / a
--     local b = (a1 * b1 + (1 - a1) * a2 * b2) / a
--     -- a = a * 256
--     return { r, g, b, a }
-- end

return UtilModule
