#pragma once
#include "alertlib.h"

namespace alert {

	// Hold relevant info for one of the gratings in a plaid.
	class ARPlaidSubGr
	{
		double m_calpha, m_salpha;
		double m_x, m_y, m_w, m_h;
		double m_contrast;
		double m_sf;
		double m_tf;
		double m_oriDeg;
	public:
		ARPlaidSubGr(double x = 0, double y = 0, double w = 0, double h = 0, double c = 100, double sf = .1, double tf = 0, double ori = 0);
		~ARPlaidSubGr() {};

		// pass alpha in degrees, this sets calpha, salpha
		void setRotDeg(double rotDeg);
		double calpha() const { return m_calpha; }
		double salpha() const { return m_salpha; }
		double sf() const { return m_sf; }
		double contrast() const { return m_contrast;  }
	};

	std::ostream& operator<<(std::ostream& out, const ARPlaidSubGr& g);

	class ARPlaidSpec : public ARSpec
	{
		ARPlaidSubGr m_g1;
		ARPlaidSubGr m_g2;
		double m_contrast;
		double m_oriDegrees;
		double m_tf;
		double m_x, m_y, m_w, m_h;

		void getDriftPos(double t, double ppd, double tf, const alert::ARPlaidSubGr& gr1, const alert::ARPlaidSubGr& gr2, double& xpos, double& ypos);
		double getGrPhase(double x, double y, double p, const ARPlaidSubGr& gr);
		void getBB(double *ll, double *ur, int W, int H, double xdr, double ydr);
		void compareBBPoint(double *ll, double *ur, double x, double y);
		void drawPlaidArea(double *ll, double *ur, double ppd, double xOrgVSG, double yOrgVSG, const alert::ARPlaidSubGr& gr1, const alert::ARPlaidSubGr& gr2);

	public:
		ARPlaidSpec(double x=0, double y=0, double w=0, double h=0) : m_g1(), m_g2(), m_contrast(100), m_oriDegrees(0), m_x(x), m_y(y), m_w(w), m_h(h) {}
		ARPlaidSpec(const ARPlaidSubGr& g1, const ARPlaidSubGr& g2, int contrast, double tf, double oriDeg, double x, double y, double w, double h) : m_g1(g1), m_g2(g2), m_contrast(contrast), m_tf(tf), m_oriDegrees(oriDeg), m_x(x), m_y(y), m_w(w), m_h(h) {}
		ARPlaidSpec(const ARPlaidSpec& plaid) : ARSpec(plaid), m_g1(plaid.g1()), m_g2(plaid.g2()), m_contrast(plaid.contrast()), m_oriDegrees(plaid.oriDegrees()), m_x(plaid.x()), m_y(plaid.y()), m_w(plaid.w()), m_h(plaid.h()) {}
		virtual ~ARPlaidSpec() {}

		ARPlaidSubGr& g1() { return m_g1; }
		const ARPlaidSubGr& g1() const { return m_g1; }
		void setG1(const ARPlaidSubGr& g) { m_g1 = g; }
		ARPlaidSubGr& g2() { return m_g2; }
		const ARPlaidSubGr& g2() const { return m_g2; }
		void setG2(const ARPlaidSubGr& g) { m_g2 = g; }

		double oriDegrees() const { return m_oriDegrees; }
		void setOriDegrees(double ori) { m_oriDegrees = ori; g1().setRotDeg(ori); g2().setRotDeg(ori); }

		double contrast() const { return m_contrast; }
		void setContrast(double c);

		double tf() const { return m_tf; }
		void setTF(double tf) { m_tf = tf; }

		double x() const { return m_x; }
		void setX(double x) { m_x = x; }
		double y() const { return m_y; }
		void setY(double y) { m_y = y; }
		double w() const { return m_w; }
		void setW(double w) { m_w = w; }
		double h() const { return m_h; }
		void setH(double h) { m_h = h; }

		int draw();
		int drawOverlay();
		void drawOverlayPage(double xorgVSG, double yorgVSG);
	};

}; 