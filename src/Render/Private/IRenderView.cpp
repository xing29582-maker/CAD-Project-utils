#include "IRenderView.h"
#include "RenderView.h"

using namespace cadutils;

std::shared_ptr<IRenderView> cadutils::IRenderView::createRenderView()
{
	return std::make_shared<RenderView>();
}
