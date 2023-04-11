#ifndef EXTERNALINTERFACE_H
#define EXTERNALINTERFACE_H

/*
	1. Generate all external interfaces of PTW and Logic related;
	2. Distinguish different interfaces according to different project names;
	3. Mark the common interfaces of each project;
*/

#include "data_Convert/AssetsFunction.h"
#include "data_Convert/ProgramDefine.h"
#include "PropertyData/PropertyType.h"

#include "proto/Numeric.pb.h"
#include "proto/Common.pb.h"
#include "proto/Scenegraph.pb.h"
#include "proto/HmiWidget.pb.h"
#include "proto/HmiBase.pb.h"
#include <google/protobuf/text_format.h>

namespace raco::dataConvert {
class ExternalInterface {
public:
	ExternalInterface();

	void EllieExInterface(HmiWidget::TWidget* widget);
	void getSlide(HmiWidget::TExternalModelParameter* ExSlide, std::string key, float value);
	void getOffset(HmiWidget::TExternalModelParameter* exTarOffset, std::string key, Vec2 value);
	void internalIndexValueVec2(HmiWidget::TInternalModelParameter* inVec2, std::string key, unsigned int index, std::string subKey, TEProviderSource src);

	void slideTargetStartOffset(HmiWidget::TWidget* widget);
	void createSlideNode(HmiWidget::TWidget* widget);
	HmiWidget::TUniform createSlideUniform();
	bool isFunctionIcon(std::string proName);
	void addEx2FunctionIcon(HmiWidget::TWidget* widget);

	void SnoozingUniqueExInterface(HmiWidget::TWidget* widget, bool addTrigger);
	void addExCompositionAnimation(HmiWidget::TWidget* widget,bool addTrigger);
	void addExLoopAnimation(HmiWidget::TWidget* widget,bool addTrigger);

private:
	AssetsFunction assetFunction_;
};

}  // namespace raco::dataConvert

#endif // EXTERNALINTERFACE_H
