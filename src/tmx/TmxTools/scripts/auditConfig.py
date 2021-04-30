#!/usr/bin/env python

import sys;
import json;
import os;
import subprocess;

tmxctl = "tmxctl";
failed = 0;

def rebuild_sysconfig(name, cfg):
	newJson = { }
	
	for item in cfg:
		newJson[item['name']] = item

	return newJson;

def set_param(name, param, value, default, description):
    option = " --set";
    if (name == "SystemConfig"):
        option = " --set-system";
        
    sys.stderr.write(tmxctl + option +
                     " --key " + json.dumps(param) + 
                     " --value " + json.dumps(value) + 
                     " --defaultValue " + json.dumps(default) +
                     " --description " + json.dumps(description));
                     
    if (name != "SystemConfig"):
        sys.stderr.write(" " + json.dumps(name));
    
    sys.stderr.write("\n");
                
def fail(name, param, cfg):
    global failed
    failed += 1;
    
    descr = "";
    if ('description' in cfg):
        descr = cfg['description'];
        
    set_param(name, param, cfg['value'], cfg['defaultValue'], descr);

def audit(a, b):
	return (json.dumps(a) == json.dumps(b));

def audit_values(name, param, cfg, currentcfg):
    checks = [ 'value', 'defaultValue' ];
    if (name != "SystemConfig"):
        checks.append('description')
    
    # Check if the current value matches the expected
    # Check if the current default value is different
    # Check if the current default description is different
    # Note that nothing can be done to repair the last two since it came from the manifest,
    # but report the error anyway
    for check in checks:
        if (not audit(cfg[check], currentcfg[check])):
            fail(name, param, cfg);
            return 1;
    
    return 0;

def audit_plugin(name, cfg):
    option = "--config"
    
    if (name == "SystemConfig"):
        option = "--system-config"
        
    jsonout = subprocess.check_output([tmxctl, option, "--json", name]);
    if (len(jsonout) > 0):
        currentcfg = json.loads(jsonout.decode("utf-8"));
    else:
        currentcfg = {};

    if (name == "SystemConfig"):
        currentcfg[name] = rebuild_sysconfig(name, currentcfg[name]);
    
    for param in cfg:
        # Check if this default key is missing from the current config
        if ((not name in currentcfg) or (not param in currentcfg[name])):
            fail(name, param, cfg[param]);
            continue;
        
        if (not audit_values(name, param, cfg[param], currentcfg[name][param])):
            continue;

    if (not name in currentcfg):
        return;
                
    for param in currentcfg[name]:
        # Check if this current key is an additional config value
        if (not param in cfg):
            sys.stderr.write("WARNING: key " + param + " not expected for plugin " + json.dumps(name) + "\n");

for file in sys.argv[1:]:
	f = open(file);
	jsonObj = json.load(f);
	for entry in jsonObj:
		# Special case handling
		if (entry == "SystemConfig"):
			jsonObj[entry] = rebuild_sysconfig(entry, jsonObj[entry]);
                     
		audit_plugin(entry, jsonObj[entry]) 

sys.exit(failed);

