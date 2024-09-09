from widgets.playing_field.field_objects.field_object import FieldObject

class FieldObjects(FieldObject, list):
    def __init__(self, name=""):
        super(FieldObjects, self).__init__(name)
