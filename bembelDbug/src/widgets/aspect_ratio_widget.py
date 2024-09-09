from PyQt5.QtWidgets import (
    QWidget,
    QBoxLayout,
    QSpacerItem,
)


class AspectRatioWidget(QWidget):
    """
    Displays a widget with a fixed aspect ratio.

    Example: Display an image with 4/3 ratio
        ic = ImageCanvas(self)
        arw = AspectRatioWidget(ic, 4, 3, self)
    """

    def __init__(self, widget, width, height, parent=None):
        """
        :param widget: Widget that should be displayed with fixed aspect ratio.
        :param width: Ratio width
        :param height: Ratio height
        :param parent: Parent widget
        """
        QWidget.__init__(self, parent)
        self.arWidth = width
        self.arHeight = height
        self.layout = QBoxLayout(QBoxLayout.LeftToRight, self)
        self.layout.addItem(QSpacerItem(0, 0))
        self.layout.addWidget(widget)
        self.layout.addItem(QSpacerItem(0, 0))

    def resizeEvent(self, event):
        newSize = event.size()
        thisAspectRatio = newSize.width() / newSize.height()

        if thisAspectRatio > self.arWidth / self.arHeight:
            self.layout.setDirection(QBoxLayout.LeftToRight)
            widgetStretch = self.height() * (self.arWidth / self.arHeight)
            outerStretch = (self.width() - widgetStretch) / 2 + 0.5
        else:
            self.layout.setDirection(QBoxLayout.TopToBottom)
            widgetStretch = self.width() * (self.arHeight / self.arWidth)
            outerStretch  = (self.height() - widgetStretch) / 2 + 0.5

        self.layout.setStretch(0, int(outerStretch))
        self.layout.setStretch(1, int(widgetStretch))
        self.layout.setStretch(2, int(outerStretch))
