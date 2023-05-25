local DataModule = {}

local G = {}

-- init module
function DataModule.init(n, v)
    G[n] = v
end

function DataModule.getData()
    return G.LuaJsonDataModule.getData()
end

function DataModule.getActiveAnimationConfig()
    return DataModule.getData().animation[DataModule.getActiveAnimationName()]
end

local globalActiveAnimaName = nil

function DataModule.getActiveAnimationName()
    if globalActiveAnimaName == nil then
        return DataModule.getData().animation.active_animation
    end
    return globalActiveAnimaName
end


function DataModule.setActiveAnimationName(an)
    globalActiveAnimaName = an
end

function DataModule.getNode()
    return DataModule.getData().node
end

-- Get uniform default values
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

-- active animation map
function DataModule.getInterfaceDefinedMap()
    local interfaceDefined = {}
    local defaultValues = {}

    function DataModule.recursionNode(node)
        if type(node.child) == 'table' and #node.child > 0 then
            for k, n in ipairs(node.child) do
                DataModule.recursionNode(n)
            end
        end

        if node.curveBinding ~= nil
            and node.curveBinding ~= nil
            and (node.basicProperty ~= nil or node.Uniform ~= nil)
        then
            for curveBindingName, bi in pairs(node.curveBinding) do
                for i, relation in ipairs(bi) do
                    local isUniforms = G.UtilModule.startWith(relation.property, 'uniforms')
                    if isUniforms then
                        if node.Uniform == nil then
                            print('warning: uniform is nil')
                        else
                            local propertySplit = G.UtilModule.split(relation.property, '.')
                            local prop = propertySplit[2]
                            local flag = propertySplit[3]
                            local outputName = node.name .. '.' .. prop
                            if type(node.Uniform) == 'nil' then
                                -- UtilModule.printJson(node)
                                print('interface defined uniforms is nil')
                            end
                            local valueType, uniformValue = DataModule.getUniformProperty(node.Uniform, prop)

                            if uniformValue == nil or valueType == nil then
                                print('get active animation cuve not found uniform: ' .. relation.property)
                                valueType = 'null'
                            end

                            if interfaceDefined[outputName] == nil then
                                interfaceDefined[outputName] = {
                                    type = valueType,
                                    isUniforms = true,
                                    objectId = node.objectId,
                                    nodeName = node.name,
                                    property = propertySplit[1] .. '.' .. prop,
                                    value = {}
                                }
                            end

                            local tmp = {
                                curveName = {
                                    [curveBindingName] = relation.curve
                                },
                                property = propertySplit[1] .. '.' .. prop,
                            }
                            if flag ~= nil then
                                if interfaceDefined[outputName].value[flag] == nil then
                                    interfaceDefined[outputName].value[flag] = tmp
                                else
                                    interfaceDefined[outputName].value[flag].curveName[curveBindingName] = relation
                                        .curve
                                end
                            elseif valueType == 'double' then
                                if interfaceDefined[outputName].value.x == nil then
                                    interfaceDefined[outputName].value.x = tmp
                                else
                                    interfaceDefined[outputName].value.x.curveName[curveBindingName] = relation.curve
                                end
                            end

                            ------------- default values --------------------------------
                            if defaultValues[outputName] == nil then
                                defaultValues[outputName] = {
                                    isBaseProperty = false,
                                    isUniforms = true,
                                    value = {
                                        __default__ = uniformValue
                                    },
                                    type = valueType
                                }
                            end
                            defaultValues[outputName].value[curveBindingName] = uniformValue
                        end
                    elseif G.UtilModule.startWith(relation.property, 'rotation')
                        or G.UtilModule.startWith(relation.property, 'scaling')
                        or G.UtilModule.startWith(relation.property, 'translation') then
                        local propertySplit = G.UtilModule.split(relation.property, '.')
                        local prop = propertySplit[1]
                        local outputName = node.name .. '.' .. prop
                        local flag = propertySplit[2]

                        if interfaceDefined[outputName] == nil then
                            interfaceDefined[outputName] = {
                                property = prop,
                                type = 'vec3f',
                                isUniforms = false,
                                objectId = node.objectId,
                                nodeName = node.name,
                                value = {}
                            }
                        end

                        if interfaceDefined[outputName].value[flag] == nil then
                            interfaceDefined[outputName].value[flag] = {
                                curveName = {
                                    [curveBindingName] = relation.curve
                                },
                                property = relation.property,
                            }
                        else
                            interfaceDefined[outputName].value[flag].curveName[curveBindingName] = relation.curve
                        end
                        -------------- default values ----------------------------
                        if defaultValues[outputName] == nil then
                            defaultValues[outputName] = {
                                isBaseProperty = true,
                                isUniforms = false,
                                value = {
                                    __default__ = node.basicProperty[prop]
                                },
                                type = 'vec3f'
                            }
                        end
                        defaultValues[outputName].value[curveBindingName] = node.basicProperty[prop]
                    else
                        error('get active animation cuve err: unknown property' .. relation.property)
                    end
                end
            end
        end
    end

    DataModule.recursionNode(DataModule.getNode())

    return interfaceDefined, defaultValues
end

-- Get animation list
function DataModule.getAnimationNames()
    local tmp = {}

    for k in pairs(DataModule.getData().animation) do
        if k ~= '' and k ~= 'Slide' and k ~= 'active_animation' then
            G.UtilModule.push(tmp, k)
        end
    end
    return tmp
end

-- transform data
function DataModule.getTransformCurveData(interfaceDefinedMap)
    local data = {}
    local activeAnimaName = DataModule.getActiveAnimationName()

    local curveMap = {}
    for i, curveItem in ipairs(DataModule.getData().curve) do
        curveMap[curveItem.name] = curveItem
    end

    -- do return data end
    for outputName, curveItem in pairs(interfaceDefinedMap) do
        data[outputName] = {
            type = curveItem.type
        }

        if curveItem.type == 'double' then
            local active = curveItem.value.x.curveName[activeAnimaName]
            if active ~= nil and curveMap[active] ~= nil then
                data[outputName].value = {
                    x = curveMap[active].pointList
                }
            else
                data[outputName].value = {
                    x = {}
                }
            end
        elseif curveItem.type == 'vec2f' or curveItem.type == 'vec3f' or curveItem.type == 'vec4f' then
            data[outputName].value = {
                x = {},
                y = {}
            }

            if curveItem.value.x ~= nil then
                local active = curveItem.value.x.curveName[activeAnimaName]
                if active ~= nil then
                    data[outputName].value.x = curveMap[active].pointList
                end
            end

            if curveItem.value.y ~= nil then
                -- UtilModule.printJson(curveItem)
                local active = curveItem.value.y.curveName[activeAnimaName]
                if active ~= nil then
                    data[outputName].value.y = curveMap[active].pointList
                end
            end

            if curveItem.type == 'vec3f' or curveItem.type == 'vec4f' then
                if curveItem.value.z ~= nil then
                    local active = curveItem.value.z.curveName[activeAnimaName]
                    if active ~= nil then
                        data[outputName].value.z = curveMap[active].pointList
                    else
                        data[outputName].value.z = {}
                    end
                else
                    data[outputName].value.z = {}
                end

                if curveItem.type == 'vec4f' then
                    if curveItem.value.w ~= nil then
                        local active = curveItem.value.w.curveName[activeAnimaName]
                        if active ~= nil then
                            data[outputName].value.w = curveMap[active].pointList
                        else
                            data[outputName].value.w = {}
                        end
                    else
                        data[outputName].value.w = {}
                    end
                end
            end
        end
    end
    return data
end

return DataModule
