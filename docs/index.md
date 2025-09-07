---
tags:
  - plugin
resource_link: "https://www.redguides.com/community/resources/mq2tributemanager.89/"
support_link: "https://www.redguides.com/community/threads/mq2tributemanager.15150/"
repository: "https://github.com/RedGuides/MQ2TributeManager"
config: "<server>_<character>.ini"
authors: "alt228, Sym, wired420"
tagline: "Adds a /tribute command in game for tribute management."
acknowledgements: "alt228 for creating this for RedGuides"
---

# MQ2TributeManager

<!--desc-start-->
This original plugin adds a /tribute command with automatic tribute status management capabilities.
<!--desc-end-->

## Commands

<a href="cmd-tribute/">
{% 
  include-markdown "projects/mq2tributemanager/cmd-tribute.md" 
  start="<!--cmd-syntax-start-->" 
  end="<!--cmd-syntax-end-->" 
%}
</a>
:    {% include-markdown "projects/mq2tributemanager/cmd-tribute.md" 
        start="<!--cmd-desc-start-->" 
        end="<!--cmd-desc-end-->" 
        trailing-newlines=false 
     %} {{ readMore('projects/mq2tributemanager/cmd-tribute.md') }}

## Settings

Example from server_charname.ini,

```ini
[MQ2TributeManager]
Mode=3
```

`Mode=1`
:   Toggle tribute on or off based on combat state automatically.

`Mode=2`
:   Toggle tribute off immediately ignoring any remaining time left.

`Mode=3`
:   Toggle tribute on or off if you (or your group) is facing a named mob.
