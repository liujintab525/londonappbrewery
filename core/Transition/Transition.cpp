#include "..\ebase.h"
#include "..\etransition.h"
#include "..\enode.h"

e2d::Transition::Transition(double duration)
	: m_bEnd(false)
	, m_fLast(0)
	, m_fRateOfProgress(0)
	, m_pPrevScene(nullptr)
	, m_pNextScene(nullptr)
	, m_pPrevLayer(nullptr)
	, m_pNextLayer(nullptr)
	, m_sPrevLayerParam()
	, m_sNextLayerParam()
{
	m_fDuration = max(duration, 0);
}

e2d::Transition::~Transition()
{
	SafeRelease(&m_pPrevScene);
	SafeRelease(&m_pNextScene);
	SafeReleaseInterface(&m_pPrevLayer);
	SafeReleaseInterface(&m_pNextLayer);
}

bool e2d::Transition::isEnding()
{
	return m_bEnd;
}

void e2d::Transition::_init(Scene * prev, Scene * next)
{
	// 创建图层
	HRESULT hr = Renderer::getRenderTarget()->CreateLayer(&m_pNextLayer);

	if (SUCCEEDED(hr))
	{
		hr = Renderer::getRenderTarget()->CreateLayer(&m_pPrevLayer);
	}

	if (FAILED(hr))
	{
		ASSERT(false, L"Create layer failed!");
	}

	m_fLast = Time::getTotalTime();
	m_pPrevScene = prev;
	m_pNextScene = next;
	if (m_pPrevScene) m_pPrevScene->retain();
	if (m_pNextScene) m_pNextScene->retain();

	m_WindowSize = Window::getSize();
	m_sPrevLayerParam = m_sNextLayerParam = D2D1::LayerParameters();
}

void e2d::Transition::_update()
{
	// 计算动画进度
	if (m_fDuration == 0)
	{
		m_fRateOfProgress = 1;
	}
	else
	{
		m_fRateOfProgress = min((Time::getTotalTime() - m_fLast) / m_fDuration, 1);
	}

	this->_updateCustom();

	// 更新场景内容
	if (m_pPrevScene)
	{
		m_pPrevScene->_update();
	}
	if (m_pNextScene)
	{
		m_pNextScene->_update();
	}
}

void e2d::Transition::_render()
{
	auto pRT = Renderer::getRenderTarget();
	Size windowSize = Window::getSize();
	if (m_pPrevScene)
	{
		Point rootPos = m_pPrevScene->getRoot()->getPos();
		auto clipRect = D2D1::RectF(
			float(max(rootPos.x, 0)),
			float(max(rootPos.y, 0)),
			float(min(rootPos.x + windowSize.width, windowSize.width)),
			float(min(rootPos.y + windowSize.height, windowSize.height))
		);
		pRT->SetTransform(D2D1::Matrix3x2F::Identity());
		pRT->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		pRT->PushLayer(m_sPrevLayerParam, m_pPrevLayer);

		// 渲染场景
		m_pPrevScene->_render();

		pRT->PopLayer();
		pRT->PopAxisAlignedClip();
	}

	if (m_pNextScene)
	{
		Point rootPos = m_pNextScene->getRoot()->getPos();
		auto clipRect = D2D1::RectF(
			float(max(rootPos.x, 0)),
			float(max(rootPos.y, 0)),
			float(min(rootPos.x + windowSize.width, windowSize.width)),
			float(min(rootPos.y + windowSize.height, windowSize.height))
		);
		pRT->SetTransform(D2D1::Matrix3x2F::Identity());
		pRT->PushAxisAlignedClip(clipRect, D2D1_ANTIALIAS_MODE_PER_PRIMITIVE);
		pRT->PushLayer(m_sNextLayerParam, m_pNextLayer);

		// 渲染场景
		m_pNextScene->_render();

		pRT->PopLayer();
		pRT->PopAxisAlignedClip();
	}
}

void e2d::Transition::_stop()
{
	m_bEnd = true;
	_reset();
}
