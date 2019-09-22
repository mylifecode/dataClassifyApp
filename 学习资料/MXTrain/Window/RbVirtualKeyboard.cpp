#include "RbVirtualKeyboard.h"
#include <QDesktopWidget>
#include "MxDefine.h"


RbVirtualKeyboard::RbVirtualKeyboard()
:QWidget(NULL,Qt::WindowStaysOnTopHint)
{
	ui.setupUi(this);


	m_signalMapper = new QSignalMapper(this);
	setMapper();
	connectMapper();
	connect(m_signalMapper, SIGNAL(mapped(const QString&)), this, SLOT(setDispText(const QString&)));
	connect(ui.toolButton_backspace, SIGNAL(clicked()), this, SLOT(onBackspace()));
	connect(ui.toolButton_clear,SIGNAL(clicked()),this,SLOT(clear()));
	Mx::setWidgetStyle(this,"qss:RbVirtualKeyboard.qss");
}

RbVirtualKeyboard::~RbVirtualKeyboard()
{

}

void RbVirtualKeyboard::setMapper()
{
	//number
	m_signalMapper->setMapping(ui.toolButton_0, ui.toolButton_0->text());
	m_signalMapper->setMapping(ui.toolButton_1, ui.toolButton_1->text());
	m_signalMapper->setMapping(ui.toolButton_2, ui.toolButton_2->text());
	m_signalMapper->setMapping(ui.toolButton_3, ui.toolButton_3->text());
	m_signalMapper->setMapping(ui.toolButton_4, ui.toolButton_4->text());
	m_signalMapper->setMapping(ui.toolButton_5, ui.toolButton_5->text());
	m_signalMapper->setMapping(ui.toolButton_6, ui.toolButton_6->text());
	m_signalMapper->setMapping(ui.toolButton_7, ui.toolButton_7->text());
	m_signalMapper->setMapping(ui.toolButton_8, ui.toolButton_8->text());
	m_signalMapper->setMapping(ui.toolButton_9, ui.toolButton_9->text());
	//letter
	m_signalMapper->setMapping(ui.toolButton_a, ui.toolButton_a->text());
	m_signalMapper->setMapping(ui.toolButton_b, ui.toolButton_b->text());
	m_signalMapper->setMapping(ui.toolButton_c, ui.toolButton_c->text());
	m_signalMapper->setMapping(ui.toolButton_d, ui.toolButton_d->text());
	m_signalMapper->setMapping(ui.toolButton_e, ui.toolButton_e->text());
	m_signalMapper->setMapping(ui.toolButton_f, ui.toolButton_f->text());
	m_signalMapper->setMapping(ui.toolButton_g, ui.toolButton_g->text());
	m_signalMapper->setMapping(ui.toolButton_h, ui.toolButton_h->text());
	m_signalMapper->setMapping(ui.toolButton_i, ui.toolButton_i->text());
	m_signalMapper->setMapping(ui.toolButton_j, ui.toolButton_j->text());
	m_signalMapper->setMapping(ui.toolButton_k, ui.toolButton_k->text());
	m_signalMapper->setMapping(ui.toolButton_l, ui.toolButton_l->text());
	m_signalMapper->setMapping(ui.toolButton_m, ui.toolButton_m->text());
	m_signalMapper->setMapping(ui.toolButton_n, ui.toolButton_n->text());
	m_signalMapper->setMapping(ui.toolButton_o, ui.toolButton_o->text());
	m_signalMapper->setMapping(ui.toolButton_p, ui.toolButton_p->text());
	m_signalMapper->setMapping(ui.toolButton_q, ui.toolButton_q->text());
	m_signalMapper->setMapping(ui.toolButton_r, ui.toolButton_r->text());
	m_signalMapper->setMapping(ui.toolButton_s, ui.toolButton_s->text());
	m_signalMapper->setMapping(ui.toolButton_t, ui.toolButton_t->text());
	m_signalMapper->setMapping(ui.toolButton_u, ui.toolButton_u->text());
	m_signalMapper->setMapping(ui.toolButton_v, ui.toolButton_v->text());
	m_signalMapper->setMapping(ui.toolButton_w, ui.toolButton_w->text());
	m_signalMapper->setMapping(ui.toolButton_x, ui.toolButton_x->text());
	m_signalMapper->setMapping(ui.toolButton_y, ui.toolButton_y->text());
	m_signalMapper->setMapping(ui.toolButton_z, ui.toolButton_z->text());

	//other
	m_signalMapper->setMapping(ui.toolButton_dot,QString("."));
	m_signalMapper->setMapping(ui.toolButton_enter,QString("\n"));
}

void RbVirtualKeyboard::connectMapper()
{
	connect(ui.toolButton_0, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_1, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_2, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_3, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_4, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_5, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_6, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_7, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_8, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_9, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	//letter
	connect(ui.toolButton_a, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_b, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_c, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_d, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_e, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_f, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_g, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_h, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_i, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_j, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_k, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_l, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_m, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_n, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_o, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_p, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_q, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_r, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_s, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_t, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_u, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_v, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_w, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_x, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_y, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_z, SIGNAL(clicked()), m_signalMapper, SLOT(map()));

	//other
	connect(ui.toolButton_dot, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
	connect(ui.toolButton_enter, SIGNAL(clicked()), m_signalMapper, SLOT(map()));
}

void RbVirtualKeyboard::setDispText(const QString &text)
{
	m_content += text;
	emit contentChanged(m_content);
}

void RbVirtualKeyboard::onBackspace()
{
	if(m_content.size())
	{
		m_content = m_content.left(m_content.size() - 1);
		emit contentChanged(m_content);
	}
}

void RbVirtualKeyboard::clear()
{
	if(m_content.size())
	{
		m_content.clear();
		emit contentChanged(m_content);
	}
}

void RbVirtualKeyboard::showEvent(QShowEvent *)
{
	m_content.clear();
	QDesktopWidget dw;
	QSize size = dw.screen(dw.primaryScreen())->size();
	move((size.width() - width()) / 2,
		size.height() - height() - 40);
}