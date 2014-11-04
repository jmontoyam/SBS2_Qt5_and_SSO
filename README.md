SBS2_Qt5_and_SSO
================

This repository is an update of the smartphonebrainscanner2 (SBS2) project 
(https://github.com/SmartphoneBrainScanner/smartphonebrainscanner2-core/wiki)
to Qt5. In this repository you can find a file called "Steps_to_Migrate_SBS2_to_Qt5.txt",
which describes all the steps involved in updating SBS2 from Qt4 to Qt5.
Additionally, in the "smartphonebrainscanner2_updated_to_Qt5_and_using_SSO/apps" folder, you can find an application that uses the SSO library (https://github.com/jmontoyam/SSO) to solve the EEG inverse problem using structured sparsity
optimization methods. Using SBS2 in combination with the SSO library, you can develop an EEG inverse solver based on standard regularizers, such as Lasso and Group Lasso regularizers, as well as the matrix factorization approach described in the paper "A regularized matrix factorization approach to induce structured sparse-low-rank solutions in the EEG inverse problem" (http://asp.eurasipjournals.com/content/2014/1/97). Finally, this updated version of the SBS2 project contains a new 3D brain visualization class, which is based on the new QWindow and QOpenGL* classes.

To see some pictures of an EEG inverse solver in action (developed using the SBS2 project (the Qt5 version contained in this repository) and the SSO library), running on mobile devices, please take a look at the README.md file of the  SSO library (https://github.com/jmontoyam/SSO/blob/master/README.md).
