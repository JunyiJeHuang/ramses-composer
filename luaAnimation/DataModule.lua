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

-- 输出名称绑定关系(一对多)
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
                    local propertySplit = G.UtilModule.split(bindItem.property, '.')
                    local prop = propertySplit[2]
                    local outputName = activeAnimaName .. '.' .. node.name .. '.' .. prop
                    local valueType, uniformValue = DataModule.getUniformProperty(node.Uniform, prop)

                    if result[outputName] == nil then
                        result[outputName] = {
                            -- objectId = node.objectId,
                            -- nodeName = node.name,
                            property = bindItem.property,
                            isUniforms = true,
                            isBaseProperty = false,
                            type = valueType,
                            nodes = {}
                        }
                    end

                    result[outputName].nodes[node.objectId] = {
                        objectId = node.objectId,
                        nodeName = node.name,
                    }
                elseif G.UtilModule.startWith(bindItem.property, 'rotation')
                    or G.UtilModule.startWith(bindItem.property, 'scale')
                    or G.UtilModule.startWith(bindItem.property, 'translation') then
                    local propertySplit = G.UtilModule.split(bindItem.property, '.')
                    local prop = propertySplit[1]
                    local outputName = activeAnimaName .. '.' .. node.name .. '.' .. prop

                    if result[outputName] == nil then
                        result[outputName] = {
                            -- objectId = node.objectId,
                            -- nodeName = node.name,
                            property = prop,
                            isUniforms = false,
                            isBaseProperty = true,
                            type = 'vec3f',
                            nodes = {}
                        }
                    end

                    result[outputName].nodes[node.objectId] = {
                        objectId = node.objectId,
                        nodeName = node.name,
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
            -- isUniforms double
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
                -- TODO: 理论上不会存在
            end
        else
            -- isBaseProperty (准确来说是组合的)
            local n = G.UtilModule.split(curveItem.name, '.')
            local outputName = G.UtilModule.join(G.UtilModule.slice(n, 1, #n - 1), '.')
            local mapping = outputNameMapping[outputName]

            if mapping ~= nil then
                local p = n[#n]
                -- TODO: 需要根据map判断具体nf
                if data[outputName] == nil then
                    data[outputName] = {
                        type = mapping.type,
                        value = {
                            x = {},
                            y = {},
                        }
                    }
                    if mapping.type == 'vec3f' then
                        data[outputName].value.z = {}
                    elseif mapping.type == 'vec4f' then
                        data[outputName].value.z = {}
                        data[outputName].value.w = {}
                    end
                end
                data[outputName].value[p] = curveItem.pointList
                -- error('unknown curve trans: '..curveItem.name)
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
function DataModule.getUniformProperty(uniform, attrName, flag)
    local uniformValue = nil
    local valueType = nil
    for i, uniformItem in ipairs(uniform) do
        if attrName == uniformItem.name then
            valueType = uniformItem.type
            if uniformItem.type == 'double' then
                uniformValue = {
                    x = uniformItem.value
                }
            elseif uniformItem.type == 'vec3f' or uniformItem.type == 'vec2f' or uniformItem.type == 'vec4f' then
                local tmp = {}
                for _i, item1 in ipairs(uniformItem.value) do
                    for k, v in pairs(item1) do
                        tmp[k] = v
                    end
                end
                uniformValue = tmp
            else
                error('unknown uniform' .. attrName)
            end
            break
        end
    end
    return valueType, uniformValue
end

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
                    local propertySplit = G.UtilModule.split(bindItem.property, '.')
                    local prop = propertySplit[2]
                    local outputName = activeAnimaName .. '.' .. node.name .. '.' .. prop
                    local valueType, uniformValue = DataModule.getUniformProperty(node.Uniform, prop)

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

                    local propertySplit = G.UtilModule.split(bindItem.property, '.')
                    local prop = propertySplit[1]
                    local outputName = activeAnimaName .. '.' .. node.name .. '.' .. prop

                    result[outputName] = {
                        isBaseProperty = true,
                        isUniforms = false,
                        value = node.basicProperty[prop],
                        type = 'vec3f'
                    }
                else
                    error('err: unknown property' .. bindItem.property)
                end
            end
        end
    end

    DataModule.recursionNode(DataModule.getNode())

    return result
end

-- 获取active animation map
function DataModule.getActiveCurveMap()
    local activeAnimationResult = {}
    local activeAnimaName = DataModule.getActiveAnimationName()

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
                    local propertySplit = G.UtilModule.split(bindItem.property, '.')
                    local prop = propertySplit[2]
                    local flag = propertySplit[3]
                    local outputName = activeAnimaName .. '.' .. node.name .. '.' .. prop
                    local valueType, uniformValue = DataModule.getUniformProperty(node.Uniform, prop)

                    if uniformValue == nil or valueType == nil then
                        error('get active animation cuve not found uniform: ' .. bindItem.property)
                    end

                    if activeAnimationResult[outputName] == nil then
                        activeAnimationResult[outputName] = {
                            prop = prop,
                            type = valueType,
                            isUniforms = true,
                            objectId = node.objectId,
                            nodeName = node.name,
                            value = {}
                        }
                    end

                    if flag ~= nil then
                        activeAnimationResult[outputName].value[flag] = {
                            curveName = bindItem.curve,
                            property = bindItem.property,
                        }
                    elseif valueType == 'double' then
                        activeAnimationResult[outputName].value.x = {
                            curveName = bindItem.curve,
                            property = bindItem.property,
                        }
                    end

                elseif G.UtilModule.startWith(bindItem.property, 'rotation')
                    or G.UtilModule.startWith(bindItem.property, 'scale')
                    or G.UtilModule.startWith(bindItem.property, 'translation') then
                    local propertySplit = G.UtilModule.split(bindItem.property, '.')
                    local prop = propertySplit[1]
                    local outputName = activeAnimaName .. '.' .. node.name .. '.' .. prop
                    local flag = propertySplit[2]

                    if activeAnimationResult[outputName] == nil then
                        activeAnimationResult[outputName] = {
                            prop = prop,
                            type = 'vec3f',
                            isUniforms = false,
                            objectId = node.objectId,
                            nodeName = node.name,
                            value = {}
                        }
                    end

                    activeAnimationResult[outputName].value[flag] = {
                        curveName = bindItem.curve,
                        property = bindItem.property,
                    }
                else
                    error('get active animation cuve err: unknown property' .. bindItem.property)
                end
            end
        end
    end

    DataModule.recursionNode(DataModule.getNode())

    return activeAnimationResult
end

function DataModule.getTransformCurveData2()
    local data = {}
    local activeCurveMap = DataModule.getActiveCurveMap()
    local outputNameMapping = DataModule.getOutputNameMapping()
    local defaultValues = DataModule.getNodeDefaultValues()
    -- 曲线可能被多次使用, 分布在不同的vecNf中
    -- 第一步转为曲线map, 方便取值

    local curveMap = {}

    for i, curveItem in ipairs(DataModule.getData().curve) do
        curveMap[curveItem.name] = curveItem
    end

    -- 根据节点找到active animation 绑定的曲线

    for outputName, curveItem in pairs(activeCurveMap) do
        data[outputName] = {
            type = curveItem.type
        }

        if curveItem.type == 'double' then
            data[outputName].value = {
                x = curveMap[curveItem.value.x.curveName].pointList
            }
        elseif curveItem.type == 'vec2f' or curveItem.type == 'vec3f' or curveItem.type == 'vec4f' then
            data[outputName].value = {
                x = {},
                y = {}
            }

            if curveItem.value.x ~= nil then
                data[outputName].value.x = curveMap[curveItem.value.x.curveName].pointList
            end

            if curveItem.value.y ~= nil then
                data[outputName].value.y = curveMap[curveItem.value.y.curveName].pointList
            end

            if curveItem.type == 'vec3f' or curveItem.type == 'vec4f' then
                if curveItem.value.z ~= nil then
                    data[outputName].value.z = curveMap[curveItem.value.z.curveName].pointList
                else
                    data[outputName].value.z = {}
                end

                if curveItem.type == 'vec4f' then
                    if curveItem.value.w ~= nil then
                        data[outputName].value.w = curveMap[curveItem.value.w.curveName].pointList
                    else
                        data[outputName].value.w = {}
                    end
                end
            end

        end
    end

    return data, outputNameMapping, defaultValues
end

return DataModule
