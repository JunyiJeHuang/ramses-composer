﻿#include "data_Convert/AssetsFunction.h"

namespace raco::dataConvert {

TIdentifier* AssetsFunction::Key(const std::string valueStr) {
	TIdentifier* key = new TIdentifier;
	key->set_valuestring(valueStr);
	return key;
}

TVariant* AssetsFunction::VariantNumeric(float Num) {
	TVariant* variant = new TVariant;
	TNumericValue* numeric = new TNumericValue;
	numeric->set_float_(Num);
	variant->set_allocated_numeric(numeric);
	return variant;
}

TVariant* AssetsFunction::VariantAsciiString(std::string str) {
	TVariant* variant = new TVariant;
	variant->set_asciistring(str);
	return variant;
}

TDataProvider* AssetsFunction::ProviderSrc(TEProviderSource value) {
	TDataProvider* provider = new TDataProvider;
	provider->set_source(value);
	return provider;
}

// curvereference
TDataProvider* AssetsFunction::ProviderCurve(std::string curveName) {
	TDataProvider* provide = new TDataProvider;
	TIdentifier* curveReference = new TIdentifier;
	curveReference->set_valuestring(curveName);
	provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
	return provide;
}

TDataProvider* AssetsFunction::ProviderNumeric(float num) {
	TDataProvider* provide = new TDataProvider;
	provide->set_allocated_variant(VariantNumeric(num));
	return provide;
}

TDataProvider* AssetsFunction::ProviderAsciiString(std::string AsciiStr) {
	TDataProvider* provider = new TDataProvider;
	provider->set_allocated_variant(VariantAsciiString(AsciiStr));
	return provider;
}

void AssetsFunction::OperandCurve(TDataBinding& Operand, std::string curveName) {
	Operand.set_allocated_provider(ProviderCurve(curveName));
}

void AssetsFunction::OperandNumeric(TDataBinding& Operand, float num) {
	Operand.set_allocated_provider(ProviderNumeric(num));
}

void AssetsFunction::OperandKeySrc(TDataBinding& Operand, std::string keyStr, TEProviderSource src) {
	Operand.set_allocated_key(Key(keyStr));
	Operand.set_allocated_provider(ProviderSrc(src));
}

void AssetsFunction::DataBindingKeyProvider(TDataBinding* Operand, std::string keyStr, TEProviderSource src) {
	Operand->set_allocated_key(Key(keyStr));
	Operand->set_allocated_provider(ProviderSrc(src));
}

void AssetsFunction::NodeParamAddIdentifier(HmiWidget::TNodeParam* nodeParam, std::string nodeName) {
	TIdentifier* identifier = new TIdentifier;
	identifier->set_valuestring(nodeName);
	nodeParam->set_allocated_identifier(identifier);
}

void AssetsFunction::ResourceParamAddIdentifier(HmiWidget::TResourceParam* resParam, std::string Name)
{
	TIdentifier* identifier = new TIdentifier;
	identifier->set_valuestring(Name);
	resParam->set_allocated_identifier(identifier);
}

void AssetsFunction::ResourceParamAddResource(HmiWidget::TResourceParam* resParam, std::string resName) {
	TDataBinding* binding = new TDataBinding;
	binding->set_allocated_provider(ProviderAsciiString(resName));
	resParam->set_allocated_resource(binding);
}

void AssetsFunction::NodeParamAddNode(HmiWidget::TNodeParam* nodeParam, std::string nodeShape) {
	TDataBinding* node = new TDataBinding;
	TDataProvider* provider = new TDataProvider;
	TVariant* variant = new TVariant;
	variant->set_asciistring(nodeShape);
	provider->set_allocated_variant(variant);
	node->set_allocated_provider(provider);
	nodeParam->set_allocated_node(node);
}

void AssetsFunction::TransformCreateScale(HmiWidget::TNodeTransform* transform, TDataBinding& operandX, TDataBinding& operandY, TDataBinding& operandZ) {
	TDataBinding* scale = new TDataBinding;
	TDataProvider* provider = new TDataProvider;
	TOperation* operation = new TOperation;
	operation->set_operator_(TEOperatorType_MuxVec3);
	operation->add_datatype(TEDataType_Float);
	operation->add_datatype(TEDataType_Float);
	operation->add_datatype(TEDataType_Float);
	// X
	auto operand = operation->add_operand();
	*operand = operandX;
	// Y
	operand = operation->add_operand();
	*operand = operandY;
	// Z
	operand = operation->add_operand();
	*operand = operandZ;

	provider->set_allocated_operation(operation);
	scale->set_allocated_provider(provider);
	transform->set_allocated_scale(scale);
}

// float(ValueStr) op float(num)
TDataBinding* AssetsFunction::BindingValueStrNumericOperatorType(std::string ValueStr, TEProviderSource src, float num, TEOperatorType op) {
	TDataBinding* binding = new TDataBinding;
	TDataProvider* provider = new TDataProvider;
	TOperation* operation = new TOperation;

	operation->set_operator_(op);
	operation->add_datatype(TEDataType_Float);
	operation->add_datatype(TEDataType_Float);

	auto operand1 = operation->add_operand();
	operand1->set_allocated_key(Key(ValueStr));
	operand1->set_allocated_provider(ProviderSrc(src));

	auto operand2 = operation->add_operand();
	operand2->set_allocated_provider(ProviderNumeric(num));
	provider->set_allocated_operation(operation);
	binding->set_allocated_provider(provider);
	return binding;
}

void AssetsFunction::CreateHmiWidgetUniform(HmiWidget::TUniform* uniform, std::string name, std::string value, TEProviderSource src) {
	TDataBinding* nameBinding = new TDataBinding;
	nameBinding->set_allocated_provider(ProviderAsciiString(name));
	uniform->set_allocated_name(nameBinding);
	TDataBinding* valueBinding = new TDataBinding;
	DataBindingKeyProvider(valueBinding, value, src);
	uniform->set_allocated_value(valueBinding);
}

void AssetsFunction::AddUniform2Appearance(HmiWidget::TAppearanceParam* appear, std::string name, std::string value, TEProviderSource src) {
	HmiWidget::TUniform uniform;
	// add OPACITY uniform
	CreateHmiWidgetUniform(&uniform, name, value, src);
	*(appear->add_uniform()) = uniform;
}

}  // namespace raco::dataConvert
