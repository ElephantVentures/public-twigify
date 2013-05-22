##Twigify##

Twigify converts Drupal 7 phpTemplate themes into Drupal 8 Twig themes :)

###Instructions###

1.  Download and install the Twigify module in a Drupal 7 installation.
2.  Enable it.
3.  Add a Drupal 7 Theme that you would like to convert to your 'sites/all/themes' directory
4. CD to the root of the Drupal Installation
5. drush cc all
6. drush cron
7. drush twigify
8. Follow the on screen instructions

###Known Issues###

####No support for .inc preprocessing files####
We have tested this module on Bartik, Stark, Zen and Omega. We have not created functionality to search for .inc that are preprocessing files - so Omega fails miserably.

####Running Converted Theme in Drupal 8####

Otherwise in Drupal 8, you can enable the theme and see it running with limited functionality (save for the error as detailed above.)
