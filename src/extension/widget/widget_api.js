
var dispatchStorageEvent = function(key, oldValue, newValue) {
  var evt = document.createEvent("CustomEvent");
  evt.initCustomEvent("storage", true, true);
  evt.key = key;
  evt.oldValue = oldValue;
  evt.newValue = newValue;
  evt.storageArea = window.widget.preference;
  document.dispatchEvent(evt);
  for (var i=0; i < window.frames.length; i++) {
    window.frames[i].document.dispatchEvent(evt);
  }
};

var widget_info_ = requireNative('WidgetModule');
var preference_ = widget_info_['preference'];
preference_.__onChanged_WRT__ = dispatchStorageEvent;

function Widget() {
  Object.defineProperties(this, {
    "author": {
      value: widget_info_["author"],
      writable: false
    },
    "description": {
      value: widget_info_["description"],
      writable: false
    },
    "name": {
      value: widget_info_["name"],
      writable: false
    },
    "shortName": {
      value: widget_info_["shortName"],
      writable: false
    },
    "version": {
      value: widget_info_["version"],
      writable: false
    },
    "id": {
      value: widget_info_["id"],
      writable: false
    },
    "authorEmail": {
      value: widget_info_["authorEmail"],
      writable: false
    },
    "authorHref": {
      value: widget_info_["authorHref"],
      writable: false
    },
    "height": {
      value: window && window.innerHeight || 0,
      writable: false
    },
    "width": {
      value: window && window.innerWidth || 0,
      writable: false
    },
    "preferences": {
      value: preference_,
      writable: false
    }
  });
};

Widget.prototype.toString = function() {
    return "[object Widget]";
};

window.widget = new Widget();
exports = Widget;
