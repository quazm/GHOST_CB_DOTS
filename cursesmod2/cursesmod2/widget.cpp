#include "widget.h"

#include "layout.h"

static uint widgetID = 0;

CWidget::CWidget(CWidget *parent, bool dummy)
{
	_parent = parent;
	_layout = 0;

	_selected = false;
	_visible = true;
	_bold = false;

	// If we haven't initialized curses yet, don't create a new curses window and panel
	if(!dummy)
	{
		_window = newwin(1, 1, 0, 0);
		_panel = new_panel(_window);
	}
	else
	{
		_window = 0;
		_panel = 0;
	}

	setMargins(1, 1, 1, 1);
	setBackgroundColor(Black);
	setForegroundColor(White);

	// Set default name. (for debugging)
	char *buf = new char[16];
	_itoa(widgetID++, buf, 16);
	_name = "Widget ";
	_name += buf;

	hide();
}

void CWidget::setParent(CWidget *parent)
{
	_parent = parent;
}

void CWidget::setPosition(uint x, uint y)
{
	_pos.set(x, y);

	if(_layout)
		_layout->setPosition(x, y);
}

CPoint CWidget::pos()
{
	return _pos;
}

void CWidget::setLayout(CLayout *layout)
{
	_layout = layout;
	_layout->setSize(_size.width(), _size.height());
	_layout->setPosition(_pos.x(), _pos.y());
}

void CWidget::setSize(uint width, uint height)
{
	_size.set(width, height);

	wresize(_window, height, width);
	replace_panel(_panel, _window);

	if(_layout)
		_layout->setSize(width, height);
}

void CWidget::setFixedSize(uint width, uint height)
{
	_size.set(width, height);
	_size.setFixed(true);

	wresize(_window, height, width);
	replace_panel(_panel, _window);

	if(_layout)
		_layout->setSize(width, height);
}

CSize CWidget::size()
{
	return _size;
}

void CWidget::hide()
{
	hide_panel(_panel);
	_visible = false;

	// Hide subwidgets too
	if(_layout)
		_layout->hide();
}

void CWidget::show()
{
	show_panel(_panel);
	_visible = true;

	// Show subwidgets too
	if(_layout)
		_layout->show();
}

void CWidget::update()
{
	if(_visible)
	{
		move_panel(_panel, _pos.y(), _pos.x());
		top_panel(_panel);

		for(uint i = 0; i < _size.width(); i++)
			for(uint j = 0; j < _size.height(); j++)
				waddch(_window, 'X');

		if(_layout)
			_layout->update();
	}
}

void CWidget::setMargins(uint top, uint bottom, uint left, uint right)
{
	_topMargin = top;
	_bottomMargin = bottom;
	_leftMargin = left;
	_rightMargin = right;
}

bool CWidget::isFocused()
{
	if(MOUSE_Y_POS >= _pos.y() && MOUSE_Y_POS < _pos.y() + _size.height() &&
	   MOUSE_X_POS >= _pos.x() && MOUSE_X_POS < _pos.x() + _size.width())
	{
		return true;
	}
	return false;
}

void CWidget::setBackgroundColor(Color color)
{
	_bgcolor = color;
	
	wbkgdset(_window, attribute(_bgcolor, _fgcolor, _bold));
	wattr_set(_window, _bold ? A_BOLD : A_NORMAL, colorpair(_bgcolor, _fgcolor), 0);
}

void CWidget::setForegroundColor(Color color)
{
	_fgcolor = color;

	wbkgdset(_window, attribute(_bgcolor, _fgcolor, _bold));
	wattr_set(_window, _bold ? A_BOLD : A_NORMAL, colorpair(_bgcolor, _fgcolor), 0);
}

void CWidget::setBold(bool bold)
{
	_bold = bold;

	wbkgdset(_window, attribute(_bgcolor, _fgcolor, _bold));
	wattr_set(_window, _bold ? A_BOLD : A_NORMAL, colorpair(_bgcolor, _fgcolor), 0);
}

Color CWidget::backgroundColor()
{
	return _bgcolor;
}

Color CWidget::foregroundColor()
{
	return _fgcolor;
}

bool CWidget::bold()
{
	return _bold;
}

CTextEdit::CTextEdit(CWidget *parent)
{
}