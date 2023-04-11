#include "data_Convert/ExternalInterface.h"
namespace raco::dataConvert {
ExternalInterface::ExternalInterface() {
}

void ExternalInterface::EllieExInterface(HmiWidget::TWidget* widget) {
	slideTargetStartOffset(widget);
}


// snoozing Unique ExInterface
void ExternalInterface::SnoozingUniqueExInterface(HmiWidget::TWidget* widget, bool addTrigger) {
	// External： CompositionAnimation, LoopAnimation
	addExCompositionAnimation(widget, addTrigger);

	// External： LoopAnimation
	addExLoopAnimation(widget, addTrigger);
}

bool ExternalInterface::isFunctionIcon(std::string proName) {
	if (proName == PRO_SNOOZING || proName == PRO_GASSTATION) {
		return true;
	}
	return false;
}

void ExternalInterface::addExCompositionAnimation(HmiWidget::TWidget* widget, bool addTrigger) {
	std::string ptwExCompositionanimation;
	if (addTrigger) {
		ptwExCompositionanimation = "i" + PTW_EX_COMPOSITIONANIMATION;
	} else {
		ptwExCompositionanimation = PTW_EX_COMPOSITIONANIMATION;
	}
	HmiWidget::TExternalModelParameter* externalModelValue = widget->add_externalmodelvalue();
	externalModelValue->set_allocated_key(assetFunction_.Key(ptwExCompositionanimation));
	externalModelValue->set_allocated_variant(assetFunction_.VariantNumeric(0.0));
}

void ExternalInterface::addExLoopAnimation(HmiWidget::TWidget* widget, bool addTrigger) {
	std::string ptwExLoopAnimation;
	if (addTrigger) {
		ptwExLoopAnimation = "i" + PTW_EX_LOOPANIMATION;
	} else {
		ptwExLoopAnimation = PTW_EX_LOOPANIMATION;
	}
	HmiWidget::TExternalModelParameter* externalModelValue = widget->add_externalmodelvalue();
	externalModelValue->set_allocated_key(assetFunction_.Key(ptwExLoopAnimation));
	externalModelValue->set_allocated_variant(assetFunction_.VariantNumeric(0.0));
}

void ExternalInterface::addEx2FuncitonIcon(HmiWidget::TWidget* widget) {
	// eParam_ScrollAreaDirection
	HmiWidget::TExternalModelParameter* externalModel = widget->add_externalmodelvalue();
	assetFunction_.externalKeyVariant(externalModel, "eParam_ScrollAreaDirection", assetFunction_.VariantScrollAreaDirection(TEScrollAreaDirection::TEScrollAreaDirection_Disabled));

	// eParam_ScrollAreaFadeParameters
	externalModel = widget->add_externalmodelvalue();
	assetFunction_.externalKeyVariant(externalModel, "eParam_ScrollAreaFadeParameters", assetFunction_.VariantNumericVec4f(-10000, 1, 1, 10000));

	// eParam_ScrollAreaInverseWorldTransformation
	externalModel = widget->add_externalmodelvalue();
	TMatrix4x4f* mat4x4f = new TMatrix4x4f;
	mat4x4f->set_m11(1);
	mat4x4f->set_m22(1);
	mat4x4f->set_m33(1);
	mat4x4f->set_m44(1);
	assetFunction_.externalKeyVariant(externalModel, "eParam_ScrollAreaInverseWorldTransformation", assetFunction_.VariantNumericFloatMatrix4(mat4x4f));
}

void ExternalInterface::slideTargetStartOffset(HmiWidget::TWidget* widget) {

	HmiWidget::TExternalModelParameter* externalModelValue;
	HmiWidget::TInternalModelParameter* internalModelValue;
	TDataBinding* binding;
	// slide
	externalModelValue = widget->add_externalmodelvalue();
	getSlide(externalModelValue, PTW_EX_SLIDE_MOVE, 0.0);

	// TargetOffset
	externalModelValue = widget->add_externalmodelvalue();
	Vec2 valueTar;
	valueTar.x = 500.0;
	valueTar.y = 0.0;
	getOffset(externalModelValue, PTW_EX_TARGETOFFSET_MOVE, valueTar);

	// StartOffset
	externalModelValue = widget->add_externalmodelvalue();
	Vec2 valueStart;
	valueStart.x = 0.0;
	valueStart.y = 0.0;
	getOffset(externalModelValue, PTW_EX_STARTOFFSET_MOVE, valueStart);

	// TargetOffset.x
	internalModelValue = widget->add_internalmodelvalue();
	internalIndexValueVec2(internalModelValue, PTW_EX_TARGETOFFSET_MOVE + ".X", 0, PTW_EX_TARGETOFFSET_MOVE, TEProviderSource_ExtModelValue);

	// TargetOffset.y
	internalModelValue = widget->add_internalmodelvalue();
	internalIndexValueVec2(internalModelValue, PTW_EX_TARGETOFFSET_MOVE + ".Y", 1, PTW_EX_TARGETOFFSET_MOVE, TEProviderSource_ExtModelValue);

	// TargetOffset.x * Slide
	internalModelValue = widget->add_internalmodelvalue();
	internalModelValue->set_allocated_key(assetFunction_.Key(PTW_EX_TARGETOFFSET_MOVE + ".X-Mul-" + PTW_EX_SLIDE_MOVE));
	binding = assetFunction_.BindingFloatStrSrcStrSrcOpType(PTW_EX_TARGETOFFSET_MOVE + ".X", TEProviderSource_IntModelValue, PTW_EX_SLIDE_MOVE, TEProviderSource_ExtModelValue, TEOperatorType_Mul);
	internalModelValue->set_allocated_binding(binding);
	// TargetOffset.y * Slide
	internalModelValue = widget->add_internalmodelvalue();
	internalModelValue->set_allocated_key(assetFunction_.Key(PTW_EX_TARGETOFFSET_MOVE + ".Y-Mul-" + PTW_EX_SLIDE_MOVE));
	binding = assetFunction_.BindingFloatStrSrcStrSrcOpType(PTW_EX_TARGETOFFSET_MOVE + ".Y", TEProviderSource_IntModelValue, PTW_EX_SLIDE_MOVE, TEProviderSource_ExtModelValue, TEOperatorType_Mul);
	internalModelValue->set_allocated_binding(binding);


	// StartOffset.x
	internalModelValue = widget->add_internalmodelvalue();
	internalIndexValueVec2(internalModelValue, PTW_EX_STARTOFFSET_MOVE + ".X", 0, PTW_EX_STARTOFFSET_MOVE, TEProviderSource_ExtModelValue);

	// StartOffset.y
	internalModelValue = widget->add_internalmodelvalue();
	internalIndexValueVec2(internalModelValue, PTW_EX_STARTOFFSET_MOVE + ".Y", 1, PTW_EX_STARTOFFSET_MOVE, TEProviderSource_ExtModelValue);

	// StartOffset.x * Slide
	internalModelValue = widget->add_internalmodelvalue();
	internalModelValue->set_allocated_key(assetFunction_.Key(PTW_EX_STARTOFFSET_MOVE + ".X-Mul-" + PTW_EX_SLIDE_MOVE));
	binding = assetFunction_.BindingFloatStrSrcStrSrcOpType(PTW_EX_STARTOFFSET_MOVE + ".X", TEProviderSource_IntModelValue, PTW_EX_SLIDE_MOVE, TEProviderSource_ExtModelValue, TEOperatorType_Mul);
	internalModelValue->set_allocated_binding(binding);
	// StartOffset.y * Slide
	internalModelValue = widget->add_internalmodelvalue();
	internalModelValue->set_allocated_key(assetFunction_.Key(PTW_EX_STARTOFFSET_MOVE + ".Y-Mul-" + PTW_EX_SLIDE_MOVE));
	binding = assetFunction_.BindingFloatStrSrcStrSrcOpType(PTW_EX_STARTOFFSET_MOVE + ".Y", TEProviderSource_IntModelValue, PTW_EX_SLIDE_MOVE, TEProviderSource_ExtModelValue, TEOperatorType_Mul);
	internalModelValue->set_allocated_binding(binding);
}

void ExternalInterface::getSlide(HmiWidget::TExternalModelParameter* ExSlide,std::string key, float value) {
	ExSlide->set_allocated_key(assetFunction_.Key(key));
	ExSlide->set_allocated_variant(assetFunction_.VariantNumeric(0.0));
}

void ExternalInterface::getOffset(HmiWidget::TExternalModelParameter* exTarOffset, std::string key, Vec2 value) {
	exTarOffset->set_allocated_key(assetFunction_.Key(key));
	exTarOffset->set_allocated_variant(assetFunction_.VariantNumericVec2(value.x, value.y));
}

void ExternalInterface::internalIndexValueVec2(HmiWidget::TInternalModelParameter* inVec2, std::string key, unsigned int index, std::string subKey, TEProviderSource src) {
	inVec2->set_allocated_key(assetFunction_.Key(key));
	TDataBinding Operand1;
	assetFunction_.OperandNumericUint(Operand1,index);
	TDataBinding Operand2;
	assetFunction_.OperandKeySrc(Operand2, subKey, src);
	TDataBinding*  binding = assetFunction_.BindingTypeDemuxVec(Operand1, Operand2, TEDataType::TEDataType_UInt, TEDataType::TEDataType_Vec2);
	inVec2->set_allocated_binding(binding);
}

void ExternalInterface::createSlideNode(HmiWidget::TWidget* widget) {
	// 不能多次创建，需要在调用之前添加判断 
	HmiWidget::TNodeParam* nodeParam = widget->add_nodeparam();
	assetFunction_.NodeParamAddIdentifier(nodeParam, PTW_EX_SLIDENODE_MOVE);
	assetFunction_.NodeParamAddNode(nodeParam, PTW_EX_SLIDENODE_MOVE);
	auto transform = nodeParam->mutable_transform();

	TDataBinding Operand1;
	Operand1.set_allocated_key(assetFunction_.Key(PTW_EX_TARGETOFFSET_MOVE + ".X-Mul-" + PTW_EX_SLIDE_MOVE));
	Operand1.set_allocated_provider(assetFunction_.ProviderSrc(TEProviderSource_IntModelValue));

	TDataBinding Operand2;
	Operand2.set_allocated_provider(assetFunction_.ProviderNumeric(0));

	TDataBinding Operand3;
	Operand3.set_allocated_key(assetFunction_.Key(PTW_EX_TARGETOFFSET_MOVE + ".Y-Mul-" + PTW_EX_SLIDE_MOVE));
	Operand3.set_allocated_provider(assetFunction_.ProviderSrc(TEProviderSource_IntModelValue));

	transform->set_allocated_translation(assetFunction_.createTransRotaScale(Operand1, Operand2, Operand3));
}

HmiWidget::TUniform ExternalInterface::createSlideUniform() {
	HmiWidget::TUniform uniform;
	std::string name = PTW_FRAG_SYS_SLIDE;  // Sys_Slide
	std::string value = PTW_EX_SLIDE_MOVE;
	TEProviderSource src = TEProviderSource_ExtModelValue;
	assetFunction_.CreateHmiWidgetUniform(&uniform, name, value, src);
	return uniform;
}

}  // namespace raco::dataConvert
