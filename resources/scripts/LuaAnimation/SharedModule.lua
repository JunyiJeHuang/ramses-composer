local SharedModule = {}

local G = {}

function SharedModule.init(n, v)
    G[n] = v
end

function SharedModule.findSiblingKeyFrames(pointList, currentFrame)
    local firstPoint = pointList[1]
    local res = {}

    for pointIndex, point in ipairs(pointList) do
        if currentFrame < firstPoint.keyFrame then
            -- Before the first point
            res[1] = firstPoint
            res[2] = firstPoint
            break
        end
        local nextPointe = pointList[pointIndex + 1]
        if nextPointe == nil then
            -- Take the last point (the current point is the last point)
            if currentFrame >= point.keyFrame then
                res[1] = point
                res[2] = point
                break
            end
        else
            if currentFrame >= point.keyFrame and currentFrame < nextPointe.keyFrame then
                res[1] = point
                res[2] = nextPointe
                break
            end
        end
    end
    return res
end

function SharedModule.getDefaultValue(name, key, activeAnimaName)
    local item = G.defaultValues[name]
    if key == nil then
        error('get default val, key is null.')
    end

    if item == nil then
        print('wran: not found: ' .. name .. ', key: ' .. key)
        return 0
    end

    local currentDefault = item.value[activeAnimaName]
    -- print('--', type(currentDefault), activeAnimaName)
    if currentDefault == nil then
        return item.value.__default__[key]
    end

    return currentDefault[key]
end

-- Obtain a list of keyframes before and after the current frame
-- Obtain the default value and the keyframe values where x, y, and z are located. If not, return an empty list
function SharedModule.getCurrentPoints(curve, currentFrame, activeAnimaName)
    -- print(activeAnimaName, currentFrame)
    local res = {}
    for outputName, curveItem in pairs(curve) do
        -- res[outputName] = {x = [], y = [], z=[]}
        if curveItem.type ~= 'null' then
            res[outputName] = {
                type = curveItem.type,
                value = {}
            }

            for key, pointList in pairs(curveItem.value) do
                if #pointList == 0 then
                    -- print('---', outputName , key, SharedModule.getDefaultValue(outputName, key))
                    res[outputName].value[key] = {
                        default = SharedModule.getDefaultValue(outputName, key, activeAnimaName)
                    }
                else
                    local siblings = SharedModule.findSiblingKeyFrames(pointList, currentFrame)
                    if #siblings == 0 then
                        res[outputName].value[key] = {
                            default = SharedModule.getDefaultValue(outputName, key, activeAnimaName)
                        }
                    else
                        --  prev  next
                        res[outputName].value[key] = {
                            prev = siblings[1],
                            next = siblings[2]
                        }
                    end
                end
            end
        end
    end

    return res
end

function SharedModule.getIngerfaceActions(curve)
    local res = {}
    for index, curveItem in ipairs(curve) do
        local n = G.UtilModule.split(curveItem.name, '.')
        if n ~= nil then
            res[index] = G.UtilModule.join(G.UtilModule.slice(n, 1, #n - 1), '.')
        end
    end
    return res
end

return SharedModule
