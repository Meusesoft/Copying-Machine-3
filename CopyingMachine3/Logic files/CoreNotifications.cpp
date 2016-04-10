#include "stdafx.h"
#include "CoreNotifications.h"

CCoreNotifications::CCoreNotifications() {

	}

CCoreNotifications::~CCoreNotifications() {

	//clean up
	oNotifications.clear();
	oSubscriptions.clear();
	}

//this function returns the number of notifications present for the give subscriber
int 
CCoreNotifications::NotificationCount(HWND phWnd) {


	int iReturn;
	int iIndex;

	iIndex = (int)oNotifications.size()-1;
	iReturn = 0;

	//iterate through the vector and count
	while (iIndex > 0) {

		if (oNotifications[iIndex].hSubscriber == phWnd) iReturn++;

		iIndex--;	
		}

	return iReturn;
	}

//this function returns the number of the given notification present for the give subscriber
int 
CCoreNotifications::NotificationSpecificCount(HWND phWnd, eCoreNotification peNotification) {

	int iReturn;
	int iIndex;

	iIndex = (int)oNotifications.size()-1;
	iReturn = 0;

	//iterate through the vector and count
	while (iIndex > 0) {

		if (oNotifications[iIndex].hSubscriber == phWnd &&
		    oNotifications[iIndex].eNotification == peNotification) iReturn++;


		iIndex--;	
		}

	return iReturn;
	}

//this function returns the first notification to the subscriber
bool 
CCoreNotifications::GetNotification(HWND phWnd, sCoreNotification& pcNotification) {

	bool bReturn;
	int  iIndex;
	int  iSize;
	
	iSize = (int)oNotifications.size();
	iIndex = 0;
	bReturn = false;

	//iterate through the vector and find the first notification for the subscriber
	while (iIndex < iSize && !bReturn) {

		if (oNotifications[iIndex].hSubscriber == phWnd) {

			//we found a notification
			CopyMemory(&pcNotification, &oNotifications[iIndex], sizeof(sCoreNotification));

			oNotifications.erase(oNotifications.begin() + iIndex);			
			bReturn = true;
			}

		iIndex++;
		}

	return bReturn;
	}

//this functions returns the first notification of the asked notification type by the subscriber
bool 
CCoreNotifications::GetNotificationSpecific(HWND phWnd, sCoreNotification& pcNotification, eCoreNotification peNotification) {

	bool bReturn;
	int  iIndex;
	int  iSize;
	
	iSize = (int)oNotifications.size();
	iIndex = 0;
	bReturn = false;

	//iterate through the vector and find the first notification for the subscriber
	while (iIndex < iSize && !bReturn) {

		if (oNotifications[iIndex].hSubscriber == phWnd &&
			oNotifications[iIndex].eNotification == peNotification) {

			//we found a notification
			CopyMemory(&pcNotification, &oNotifications[iIndex], sizeof(sCoreNotification));

			oNotifications.erase(oNotifications.begin() + iIndex);			
			bReturn = true;
			}

		iIndex++;
		}

	return bReturn;
}

//this functions removes all notifications of the asked notification type by the subscriber
bool 
CCoreNotifications::RemoveNotificationSpecific(HWND phWnd, eCoreNotification peNotification) {

	bool bReturn;
	int  iIndex;
	int  iSize;
	
	iSize = (int)oNotifications.size();
	iIndex = 0;
	bReturn = false;

	//iterate through the vector and find the first notification for the subscriber
	while (iIndex < iSize) {

		if (oNotifications[iIndex].hSubscriber == phWnd &&
			oNotifications[iIndex].eNotification == peNotification) {

			//we found a notification
			oNotifications.erase(oNotifications.begin() + iIndex);
			iSize = (int)oNotifications.size();
			bReturn = true;
			}

		iIndex++;
		}

	return bReturn;
}

//this functions communicates the given notification to all subscribers
bool 
CCoreNotifications::CommunicateEvent(eCoreNotification peNotification, void* pData) {

	bool bReturn;
	bool bContinue;
	int iIndex;
	int iNotificationIndex;
	sCoreNotification cNotification;

	bReturn = false;

	cNotification.eNotification = peNotification;
	cNotification.pData = pData;
	

	iIndex = (int)oSubscriptions.size()-1;

	while (iIndex>=0) {

		if (oSubscriptions[iIndex].eNotification == peNotification) {

			//we found a subscriber
			cNotification.hSubscriber = oSubscriptions[iIndex].hSubscriber;

			//check if there isn't a notification waiting already
			iNotificationIndex = (int)oNotifications.size()-1;
			bContinue = true;

			while (iNotificationIndex>=0 && bContinue) {

				bContinue = !(cNotification.eNotification == oNotifications[iNotificationIndex].eNotification &&
					cNotification.hSubscriber == oNotifications[iNotificationIndex].hSubscriber &&
					cNotification.pData == oNotifications[iNotificationIndex].pData);

				iNotificationIndex--;
				}


			if (bContinue) {
				
				oNotifications.push_back(cNotification);
				}
				//send a message to the subscriber to look into the notifications waiting for it
				if (PostMessage(cNotification.hSubscriber, WM_CORENOTIFICATION, 0, 0)==0) {
					//MsgBox;
					}
				//}

			bReturn = true;
			}

		iIndex--;
		}

	return bReturn;
	}

//this functions subscribes a window to the given notification
void 
CCoreNotifications::Subscribe(HWND phWnd, eCoreNotification peNotification) {

	sSubscription cSubscription;

	cSubscription.hSubscriber = phWnd;
	cSubscription.eNotification = peNotification;

	oSubscriptions.push_back(cSubscription);
	}

//this function unsubscribes the subscriber of the given notification
bool 
CCoreNotifications::Unsubscribe(HWND phWnd, eCoreNotification peNotification) {

	bool bReturn;
	int iIndex;


	bReturn = false;
	iIndex = (int)oSubscriptions.size()-1;

	while (iIndex>=0) {

		if (oSubscriptions[iIndex].hSubscriber == phWnd &&
			oSubscriptions[iIndex].eNotification == peNotification) {

			oSubscriptions.erase(oSubscriptions.begin() + iIndex);

			bReturn = true; //we have unsubscribed something

			}

		iIndex--;
		}

	return bReturn;
	}

//this function unsubscribes the subscriber from all its subscriptions to notifications
bool 
CCoreNotifications::UnsubscribeAll(HWND phWnd) {

	bool bReturn;
	int iIndex;


	bReturn = false;
	iIndex = (int)oSubscriptions.size()-1;

	while (iIndex>=0) {

		if (oSubscriptions[iIndex].hSubscriber == phWnd) {

			oSubscriptions.erase(oSubscriptions.begin() + iIndex);

			bReturn = true; //we have unsubscribed something

			}

		iIndex--;
		}

	return bReturn;
	}
