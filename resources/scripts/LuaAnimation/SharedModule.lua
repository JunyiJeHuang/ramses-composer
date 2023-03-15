local SharedModule = {}

local G = {}

-- 初始化当前Module
function SharedModule.init(n, v)
    G[n] = v
end

-- 查找前后的兄弟关键帧
function SharedModule.findSiblingKeyFrames(pointList, currentFrame)
    local firstPoint = pointList[1]
    local res = {}

    for pointIndex, point in ipairs(pointList) do
        if currentFrame < firstPoint.keyFrame then
            -- 第一个点之前
            res[1] = firstPoint
            res[2] = firstPoint
            break
        end
        local nextPointe = pointList[pointIndex + 1]
        if nextPointe == nil then
            -- 取最后一个点(当前点为最后一个点)
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

-- 获取默认值
function SharedModule.getDefaultValue(name, key)
    local item = G.defaultValues[name]
    if key == nil then
        error('get default val, key is null.')
    end

    if item == nil then
        print('wran: not found: ' .. name .. ', key: ' .. key)
        return 0
    end

    return item.value[key]
end

-- 获取当前帧前后关键帧列表
-- TODO: 获取默认值和x,y,z分别所在的关键帧值, 如果获取不到返回一个空的列表
function SharedModule.getCurrentPoints(curve, currentFrame)
    local res = {}
    for outputName, curveItem in pairs(curve) do
        -- res[outputName] = {x = [], y = [], z=[]}
        res[outputName] = {
            type = curveItem.type,
            value = {}
        }

        for key, pointList in pairs(curveItem.value) do
            if #pointList == 0 then
                -- TODO: 这里应该可以获取默认值
                res[outputName].value[key] = {
                    default = SharedModule.getDefaultValue(outputName, key)
                }
            else
                local siblings = SharedModule.findSiblingKeyFrames(pointList, currentFrame)
                if #siblings == 0 then
                    -- TODO: 这里应该可以获取默认值
                    res[outputName].value[key] = {
                        default = SharedModule.getDefaultValue(outputName, key)
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

    return res
end

-- 获取接口列表
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
