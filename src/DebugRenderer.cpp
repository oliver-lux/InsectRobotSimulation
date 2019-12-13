#include "DebugRenderer.h"
#include "MeshPool.h"
#include "CinderImGui.h"
using namespace ci;
using namespace ci::app;
using namespace std;

void DebugRenderer::setup(NodeRef root)
{
	mRoot = root;
	
	updateCamera();
	
	

	
	gl::Texture2d::Format depthFormat;
	depthFormat.setInternalFormat(GL_DEPTH_COMPONENT32F);
	depthFormat.setCompareMode(GL_COMPARE_REF_TO_TEXTURE);
	depthFormat.setMagFilter(GL_LINEAR);
	depthFormat.setMinFilter(GL_LINEAR);
	depthFormat.setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

	depthFormat.setCompareFunc(GL_LEQUAL);
	mShadowMapTex = gl::Texture2d::create(fboSize, fboSize, depthFormat);

	gl::Fbo::Format fboFormat;
	fboFormat.attachment(GL_DEPTH_ATTACHMENT, mShadowMapTex);
	mFbo = gl::Fbo::create(fboSize, fboSize, fboFormat);



	mLightPos = vec3(500.0f, 1000.0f, 0.0f);
	mLightCam.setPerspective(60.0f, mFbo->getAspectRatio(), 10.f, 2000.0f);
	mLightCam.lookAt(mLightPos, vec3(0.0f));
	
	mGlsl = gl::GlslProg::create(loadAsset("shadow_shader.vert"), loadAsset("shadow_shader.frag"));
	floor = MP()->getMesh("floor");

	

	gl::enableDepthRead();
	gl::enableDepthWrite();

}
void  DebugRenderer::update() {

	gl::enable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(2.0f, 2.0f);

	gl::ScopedFramebuffer fbo(mFbo);
	gl::ScopedViewport viewport(vec2(0.0f), mFbo->getSize());
	gl::clear(Color::black());
	gl::color(Color::white());
	gl::setMatrices(mLightCam);
	
	mRoot->drawShadow();

	floor->drawShadow();


	gl::disable(GL_POLYGON_OFFSET_FILL);
	

}


void DebugRenderer::draw()
{
	gl::setMatrices(mCam);
	//gl::drawCoordinateFrame();

	//mRoot->draw();

	gl::ScopedTextureBind texScope(mShadowMapTex, (uint8_t)0);
	vec3 mvLightPos = vec3(gl::getModelView() * vec4(mLightPos, 1.0f));
	mat4 shadowMatrix = mLightCam.getProjectionMatrix() * mLightCam.getViewMatrix();
	

	MP()->mGlslShadow->uniform("uShadowMap", 0);
	MP()->mGlslShadow->uniform("uLightPos", mLightPos);
	MP()->mGlslShadow->uniform("uShadowMatrix", shadowMatrix);

	mRoot->draw();
	
	
	floor->draw();
	
	
}
void DebugRenderer::updateCamera()
{
	float x = cameraDistance * sinf(cameraTheta)*cosf(cameraPhi);
	float z = cameraDistance * sinf(cameraTheta)*sinf(cameraPhi);
	float y = cameraDistance * cosf(cameraTheta);

	mCam.lookAt(vec3(x, y, z), vec3(0));
	mCam.setPerspective(30, getWindowAspectRatio(), 100, 4000);
}

void DebugRenderer::showRenderWindow()
{



		ui::ScopedWindow window("Renderer");
		if (ui::DragFloat("Camera Theta", &cameraTheta,0.01,0.001,3.1415/2)) updateCamera();
		if (ui::DragFloat("Camera Phi", &cameraPhi, 0.01, 0, 3.1415)) updateCamera();
		if (ui::DragFloat("Camera distance", &cameraDistance, 1, 500, 2000)) updateCamera();

}