#include <escher/input_view_controller.h>
#include <escher/app.h>
#include <assert.h>

InputViewController::ContentView::ContentView(TextFieldDelegate * textFieldDelegate) :
  View(),
  m_mainView(nullptr),
  m_textField(nullptr, m_textBody, 255, textFieldDelegate),
  m_visibleInput(false)
{
  m_textBody[0] = 0;
}

void InputViewController::ContentView::setMainView(View * subview) {
  m_mainView = subview;
}

int InputViewController::ContentView::numberOfSubviews() const {
  if (m_visibleInput) {
    return 2;
  }
  return 1;
}

View * InputViewController::ContentView::subviewAtIndex(int index) {
  switch (index) {
    case 0:
      return m_mainView;
    case 1:
      if (numberOfSubviews() == 2) {
        return &m_textField;
      } else {
        assert(false);
        return nullptr;
      }
    default:
      assert(false);
      return nullptr;
  }
}

void InputViewController::ContentView::layoutSubviews() {
  m_mainView->setFrame(bounds());
  if (numberOfSubviews() == 2) {
    KDRect inputView(0, bounds().height() - k_textFieldHeight, bounds().width(), k_textFieldHeight);
    m_textField.setFrame(inputView);
  }
}

void InputViewController::ContentView::setVisibleInput(bool showInput) {
  m_visibleInput = showInput;
  markRectAsDirty(KDRect(0, bounds().height() - k_textFieldHeight, bounds().width(), k_textFieldHeight));
  layoutSubviews();
}

bool InputViewController::ContentView::visibleInput() {
  return m_visibleInput;
}

TextField * InputViewController::ContentView::textField() {
  return &m_textField;
}

InputViewController::InputViewController(Responder * parentResponder, ViewController * child, TextFieldDelegate * textFieldDelegate) :
  ViewController(parentResponder),
  m_contentView(textFieldDelegate),
  m_previousResponder(nullptr),
  m_regularViewController(child),
  m_successAction(Invocation(nullptr, nullptr)),
  m_failureAction(Invocation(nullptr, nullptr))
{
  m_contentView.textField()->setParentResponder(this);
}

View * InputViewController::view() {
  if (m_contentView.subviewAtIndex(0) == nullptr) {
    m_contentView.setMainView(m_regularViewController->view());
  }
  return &m_contentView;
}

const char * InputViewController::title() const {
  return "InputViewController";
}

const char * InputViewController::textBody() {
  return m_contentView.textField()->textBuffer();
}

void InputViewController::showInput(bool show) {
  m_contentView.setVisibleInput(show);
  if (show) {
    m_previousResponder = app()->firstResponder();
    app()->setFirstResponder(m_contentView.textField());
  } else {
    app()->setFirstResponder(m_previousResponder);
  }
}

void InputViewController::setTextBody(const char * text) {
  m_contentView.textField()->setTextBuffer(text);
}

bool InputViewController::handleEvent(Ion::Events::Event event) {
  if (!m_contentView.visibleInput()) {
    return false;
  }
  switch (event) {
    case Ion::Events::Event::ENTER:
      m_successAction.perform(this);
      showInput(false);
      return true;
    case Ion::Events::Event::ESC:
      m_failureAction.perform(this);
      showInput(false);
      return true;
    default:
      return false;
  }
}

void InputViewController::edit(Responder * caller, const char * initialContent, void * context, Invocation::Action successAction, Invocation::Action failureAction) {
  m_successAction = Invocation(successAction, context);
  m_failureAction = Invocation(failureAction, context);
  setTextBody(initialContent);
  showInput(true);
}
