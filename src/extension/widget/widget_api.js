
var sendSyncMessage = function(msg) {
  return JSON.parse(extension.internal.sendSyncMessage(JSON.stringify(msg)));
};


// test
/*
var backend_keys = [];
var backend_value = {};

var sendSyncMessage = function(msg) {
  if (msg['cmd'] == 'init') {
    return {'status':'ok', result:{'author':'lee seung keun', 'description':'.....test','name':'test app...'}};
  }
  if (msg['cmd'] == 'length') {
    return {'status':'ok', result:backend_keys.length};
  }
  if (msg['cmd'] == 'key') {
    var result = backend_keys[msg['args']['idx']] || null;
    var status = result ? "ok" : "error";
    return {'status':status, 'result':result};
  }
  if (msg['cmd'] == 'getItem') {
    var result = backend_value[msg['args']['key']] || null;
    var status = result ? "ok" : "error";
    return {'status':status, 'result':result};
  }
  if (msg['cmd'] == 'setItem') {
    if (backend_keys.lastIndexOf(msg['args']['key']) == -1)
      backend_keys.push(msg['args']['key']);
    var old = backend_value[msg['args']['key']] || null;
    backend_value[msg['args']['key']] = msg['args']['value'];
    return {'status':'ok', 'result':old};
  }
  if (msg['cmd'] == 'removeItem') {
    if (backend_keys.lastIndexOf(msg['args']['key']) == -1)
      return {'status':'error'};
    var old = backend_value[msg['args']['key']] || null;
    delete backend_value[msg['args']['key']];
    return {'status':'ok', 'result':old};
  }
  if (msg['cmd'] == 'clear') {
    backend_keys = [];
    backend_value = {};
    return {'status':'ok'};
  }
};
*/

var dispatchStorageEvent = function(key, oldValue, newValue) {
  var evt = new CustomEvent("Storage");
  evt.key = key;
  evt.oldValue = oldValue;
  evt.newValue = newValue;
  evt.storageArea = window.widget.preference;
  document.dispatchEvent(evt);
  for (var i=0; i < window.frames.length; i++) {
    window.frames[i].document.dispatchEvent(evt);
  }
};

var widget_info_ = sendSyncMessage({cmd:"init"})['result'];

function Preference() {
  Object.defineProperty(this, 'length', {
    get : function() {
      return sendSyncMessage({'cmd':'length'})['result'];
    }
  });
};

Preference.prototype.key = function(idx) {
  var req = {'cmd':'key'};
  req['args'] = {'idx':idx};

  var ret = sendSyncMessage(req);
  if (ret['status'] == 'error') {
    return null;
  }
  return ret['result'];
};

Preference.prototype.getItem = function(key) {
  var req = {'cmd':'getItem'};
  req['args'] = {'key':key};

  var ret = sendSyncMessage(req);
  if (ret['status'] == 'error') {
    return null;
  }
  return ret['result'];
};

Preference.prototype.clear = function() {
  var req = {'cmd':'clear'};
  sendSyncMessage(req);
  dispatchStorageEvent(null,null,null);
}

Preference.prototype.setItem = function(key, value) {
  var req = {'cmd':'setItem'};
  req['args'] = {'key':key,'value':value};
  var ret = sendSyncMessage(req);
  if (ret['status'] == 'error') {
    throw {code:22, name:'QuotaExceededError', message: "can't insert Item"};
  }
  var oldValue = ret['result'];
  dispatchStorageEvent(key,oldValue,value);
};

Preference.prototype.removeItem = function(key) {
  var req = {'cmd':'removeItem'};
  req['args'] = {'key':key};
  var ret = sendSyncMessage(req);
  var oldValue = ret['result'];
  if (ret['status'] == 'success') {
    dispatchStorageEvent(key,oldValue,null);
  }
};


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
      value: parseInt(widget_info_["height"]),
      writable: false
    },
    "width": {
      value: parseInt(widget_info_["width"]),
      writable: false
    },
    "preferences": {
      value: new Preference(),
      writable: false
    }
  });
};

window.widget = new Widget();
exports = Widget;
