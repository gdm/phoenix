{% if grains['os'] == 'Ubuntu' %}

drush:
  git.latest:
    - name: https://github.com/drush-ops/drush
    - rev: 6.3.0
    - target: /opt/drush
    # Check drush version not by calling drush --version, but in drush.info
    # so it doesn't create non-writable drush cache owned by root.
    - unless: test "6.3.0" == "`grep drush_version /opt/drush/drush.info | cut -d= -f2`"
    - require:
      - pkg: php5-pkgs
  cmd.wait:
    - name: ln -s /opt/drush/drush /usr/local/bin/drush
    - watch:
      - git: drush

drush-user-config-folder:
  file.recurse:
    - name: /home/vagrant/.drush
    - source: salt://drush
    - user:  vagrant
    - group: vagrant
    - unless:  drush --version
    - maxdepth: 0
    - require:
      - cmd: drush

drush-module-make-local:
  cmd.run:
    - name: 'drush -y dl make_local-6.x-1.0'
    - user:  vagrant
    - group: vagrant
    - unless: test -f /home/vagrant/.drush/make_local/make_local.drush.inc
    - require:
      - cmd: drush

drush-module-language:
  cmd.run:
    - name: 'drush dl drush_language'
    - user:  vagrant
    - group: vagrant
    - unless: test -f /home/vagrant/.drush/drush_language/language.drush.inc
    - require:
      - cmd: drush

drush-module-terminus:
  git.latest:
    - name: https://github.com/pantheon-systems/terminus
    - target: /home/vagrant/.drush/terminus
    - user:  vagrant
    - group: vagrant
    - unless: test -d /home/vagrant/.drush/terminus
    - require:
      - file: drush-user-config-folder
  # Nice to have: use composer plugin. Keep an eye on
  # http://docs.saltstack.com/en/latest/ref/states/all/salt.states.composer.html
  # Now it doesn't support update command, but it's already requested.
  # https://groups.google.com/forum/#!msg/salt-users/WmYMRuiHKQ8/33qoRSgKwqYJ
  cmd.wait:
    - name: composer update --no-dev
    - cwd: /home/vagrant/.drush/terminus
    - user:  vagrant
    - group: vagrant
    - watch:
      - git: drush-module-terminus
    - require:
      - sls: composer

drush-internal-cache:
  cmd.run:
    - name: drush cc drush
    - order: last
    - user:  vagrant
    - group: vagrant

{% endif %}
