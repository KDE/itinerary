// SPDX-FileCopyrightText: 2025 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtWebView
import org.kde.kirigami as Kirigami
import org.kde.kirigamiaddons.delegates as Delegates
import org.kde.itinerary as Itinerary

Kirigami.ScrollablePage {
    id: root

    title: i18nc("@title", "Integrations")

    ListView {
        currentIndex: -1
        model: Itinerary.AccountModel.accountTypeModel

        delegate: Delegates.RoundedItemDelegate {
            id: integrationDelegate

            required property int index
            required property string identifier
            required property string name
            required property string description
            required property string iconName
            required property string protocol

            text: name
            icon.name: Qt.resolvedUrl(iconName)

            contentItem: Delegates.SubtitleContentItem {
                itemDelegate: integrationDelegate
                subtitle: integrationDelegate.description
            }

            onClicked: {
                const account = Itinerary.AccountModel.create(identifier);
                account.openUrl.connect((url) => {
                    root.Kirigami.PageStack.push(webViewPage, {
                        url,
                        protocol: integrationDelegate.protocol,
                        account,
                    })
                })
            }
        }
    }

    Component {
        id: webViewPage


        Kirigami.Page {
            id: page

            property alias url: webview.url
            property string protocol
            property var account

            title: i18nc("@title", "Login")

            leftPadding: 0
            rightPadding: 0
            topPadding: 0
            bottomPadding: 0

            contentItem: WebView {
                id: webview

                onLoadingChanged: (request) => {
                    if (request.url.toString().startsWith(page.protocol)) { 
                        account.handleCallback(request.url)
                        root.Kirigami.PageStack.closeDialog();
                    }
                }
            }
        }
    }
}
