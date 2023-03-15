local DataModule = {}


local G = {}

-- G.LuaJsonDataModule

-- 初始化当前Module
function DataModule.init(n, v)
    G[n] = v
end

function DataModule.getData()
    return G.LuaJsonDataModule.getData()
end

-- 获取活动动画属性配置
function DataModule.getActiveAnimationConfig()
    return DataModule.getData().animation[DataModule.getActiveAnimationName()]
end

-- 输出名称绑定关系
function DataModule.getOutputNameMapping()
    local result = {}
    local activeAnimaName = DataModule.getActiveAnimationName()

    -- 递归解析节点
    function DataModule.recursionNode(node)
        if type(node.child) == 'table' and #node.child > 0 then
            for k, n in ipairs(node.child) do
                DataModule.recursionNode(n)
            end
        end

        if node.curveBinding ~= nil and node.curveBinding[activeAnimaName] ~= nil then
            for i, bindItem in ipairs(node.curveBinding[activeAnimaName]) do
                local isUniforms = G.UtilModule.startWith(bindItem.property, 'uniforms')
                if isUniforms then
                    local attrName = G.UtilModule.split(bindItem.property, '.')[2]
                    local valueType = DataModule.getUniformProperty(node.Uniform, attrName)

                    local outputName = bindItem.curve
                    result[outputName] = {
                        objectId = node.objectId,
                        nodeName = node.name,
                        property = bindItem.property,
                        isUniforms = true,
                        isBaseProperty = false,
                        type = valueType
                    }
                elseif G.UtilModule.startWith(bindItem.property, 'rotation')
                    or G.UtilModule.startWith(bindItem.property, 'scale')
                    or G.UtilModule.startWith(bindItem.property, 'translation') then
                    local n = G.UtilModule.split(bindItem.curve, '.')
                    local outputName = G.UtilModule.join(G.UtilModule.slice(n, 1, #n - 1), '.')
                    local attrName = G.UtilModule.split(bindItem.property, '.')[1]
                    result[outputName] = {
                        objectId = node.objectId,
                        nodeName = node.name,
                        property = attrName,
                        isUniforms = false,
                        isBaseProperty = true,
                        type = 'vec3f'
                    }
                else
                    error('err: unknown property mapping' .. bindItem.property)
                end
            end
        end
    end

    DataModule.recursionNode(DataModule.getNode())

    return result
end

function DataModule.getTransformCurveData()
    local data = {}
    local outputNameMapping = DataModule.getOutputNameMapping()
    local defaultValues = DataModule.getNodeDefaultValues()
    for i, curveItem in ipairs(DataModule.getData().curve) do
        -- TODO: 待优化
        local mapping = outputNameMapping[curveItem.name]
        if mapping ~= nil then
            -- isUniforms
            local outputName = curveItem.name
            -- print('mappingmapping', mapping.type)
            data[outputName] = {
                type = mapping.type
            }
            if mapping.type == 'double' then
                data[outputName].value = {
                    x = curveItem.pointList
                }
            elseif mapping.type == 'vec3f' then
                -- TODO: 
            end
        else
            -- isBaseProperty
            local n = G.UtilModule.split(curveItem.name, '.')
            local outputName = G.UtilModule.join(G.UtilModule.slice(n, 1, #n - 1), '.')

            if outputNameMapping[outputName] ~= nil then
                local p = n[#n]
                if data[outputName] == nil then
                    data[outputName] = {
                        type = 'vec3f',
                        value = {
                            x = {},
                            y = {},
                            z = {}
                        }
                    }
                end
                data[outputName].value[p] = curveItem.pointList
            end
        end
    end

    return data, outputNameMapping, defaultValues
end

-- 获取当前激活的动画名称
function DataModule.getActiveAnimationName()
    return DataModule.getData().animation.active_animation
end

function DataModule.getNode()
    return DataModule.getData().node
end

-- 获取uniform默认值等
function DataModule.getUniformProperty(uniform, attrName)
    local uniformValue = nil
    local valueType = nil
    for i, uniformItem in ipairs(uniform) do
        if attrName == uniformItem.name then
            valueType = uniformItem.type
            if uniformItem.type == 'double' then
                uniformValue = {
                    x = uniformItem.value
                }
            elseif uniformItem.type == 'vec3f' then
                local tmp = {}
                for _i, item1 in ipairs(uniformItem.value) do
                    for k, v in pairs(item1) do
                        tmp[k] = v
                    end
                end
                uniformValue = tmp
            end
            break
        end
    end
    return valueType, uniformValue
end

local defaultValuesCache = nil

-- 获取节点属性默认值
function DataModule.getNodeDefaultValues()
    local result = {}
    local activeAnimaName = DataModule.getActiveAnimationName()
    -- local basicPropertyNames = {'rotation', 'scale', 'translation'}
    -- 递归解析节点
    function DataModule.recursionNode(node)
        if type(node.child) == 'table' and #node.child > 0 then
            for k, n in ipairs(node.child) do
                DataModule.recursionNode(n)
            end
        end

        if node.curveBinding ~= nil
            and node.curveBinding[activeAnimaName] ~= nil
            and (node.basicProperty ~= nil or node.Uniform ~= nil)
        then
            for i, bindItem in ipairs(node.curveBinding[activeAnimaName]) do
                local isUniforms = G.UtilModule.startWith(bindItem.property, 'uniforms')
                if isUniforms then
                    local outputName = bindItem.curve
                    local attrName = G.UtilModule.split(bindItem.property, '.')[2]
                    local valueType, uniformValue = DataModule.getUniformProperty(node.Uniform, attrName)

                    -- for i, uniformItem in ipairs(node.Uniform) do
                    --     if attrName == uniformItem.name then
                    --         valueType = uniformItem.type
                    --         if uniformItem.type == 'double' then
                    --             uniformValue = uniformItem.value
                    --         elseif uniformItem.type == 'vec3f' then
                    --             local tmp = {}
                    --             for _i, item1 in ipairs(uniformItem.value) do
                    --                 for k, v in pairs(item1) do
                    --                     tmp[k] = v
                    --                 end
                    --             end
                    --             uniformValue = tmp
                    --         end
                    --         break
                    --     end
                    -- end
                    if uniformValue == nil or valueType == nil then
                        error('not found uniform: ' .. bindItem.property)
                    end
                    result[outputName] = {
                        isBaseProperty = false,
                        isUniforms = true,
                        value = uniformValue,
                        type = valueType
                    }
                elseif G.UtilModule.startWith(bindItem.property, 'rotation')
                    or G.UtilModule.startWith(bindItem.property, 'scale')
                    or G.UtilModule.startWith(bindItem.property, 'translation') then
                    local n = G.UtilModule.split(bindItem.curve, '.')
                    local outputName = G.UtilModule.join(G.UtilModule.slice(n, 1, #n - 1), '.')
                    local attrName = G.UtilModule.split(bindItem.property, '.')[1]
                    result[outputName] = {
                        isBaseProperty = true,
                        isUniforms = false,
                        value = node.basicProperty[attrName],
                        type = 'vec3f'
                    }
                else
                    error('err: unknown property' .. bindItem.property)
                end
            end
        end
    end

    if defaultValuesCache == nil then
        DataModule.recursionNode(DataModule.getNode())
        -- defaultValuesCache = result
        return result
    end

    return defaultValuesCache
end

return DataModule
