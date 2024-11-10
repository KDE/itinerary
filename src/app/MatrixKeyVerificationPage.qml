// SPDX-FileCopyrightText: 2024 Volker Krause <vkrause@kde.org>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick
import QtQuick.Controls as QQC2
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.quotient

Kirigami.ScrollablePage {
    id: root
    title: i18n("Matrix Session Verification")

    required property var session

    Component {
        id: message
        ColumnLayout {
            Kirigami.Icon {
                Layout.preferredWidth: Kirigami.Units.iconSizes.enormous
                Layout.preferredHeight: Kirigami.Units.iconSizes.enormous
                Layout.alignment: Qt.AlignHCenter
                source:  switch (root.session.state) {
                    case KeyVerificationSession.WAITINGFORREADY:
                    case KeyVerificationSession.INCOMING:
                    case KeyVerificationSession.WAITINGFORMAC:
                        return "security-medium-symbolic";
                    case KeyVerificationSession.DONE:
                        return "security-high";
                    case KeyVerificationSession.CANCELED:
                        return "security-low";
                    default:
                        return "";
                }
            }
            QQC2.Label {
                Layout.fillWidth: true
                text: {
                    switch (root.session.state) {
                    case KeyVerificationSession.WAITINGFORREADY:
                        return i18nc("Matrix session verification", "Waiting for device to accept verification.");
                    case KeyVerificationSession.INCOMING:
                        return i18nc("Matrix session verification", "Incoming key verification request from device <b>%1</b>", root.session.remoteDeviceId);
                    case KeyVerificationSession.WAITINGFORMAC:
                        return i18nc("Matrix session verification", "Waiting for other party to verify.");
                    case KeyVerificationSession.DONE:
                        return i18nc("Matrix session verification", "Successfully verified device <b>%1</b>", root.session.remoteDeviceId)
                    case KeyVerificationSession.CANCELED:
                    {
                        switch (root.reason) {
                            case KeyVerificationSession.NONE:
                                return i18nc("Matrix session verification", "The session verification was canceled for unknown reason.");
                            case KeyVerificationSession.TIMEOUT:
                                return i18nc("Matrix session verification", "The session verification timed out.");
                            case KeyVerificationSession.REMOTE_TIMEOUT:
                                return i18nc("Matrix session verification", "The session verification timed out for remote party.");
                            case KeyVerificationSession.USER:
                                return i18nc("Matrix session verification", "You canceled the session verification.");
                            case KeyVerificationSession.REMOTE_USER:
                                return i18nc("Matrix session verification", "The remote party canceled the session verification.");
                            case KeyVerificationSession.UNEXPECTED_MESSAGE:
                                return i18nc("Matrix session verification", "The session verification was canceled because we received an unexpected message.");
                            case KeyVerificationSession.REMOTE_UNEXPECTED_MESSAGE:
                                return i18nc("Matrix session verification", "The remote party canceled the session verification because it received an unexpected message.");
                            case KeyVerificationSession.UNKNOWN_TRANSACTION:
                                return i18nc("Matrix session verification", "The session verification was canceled because it received a message for an unknown session.");
                            case KeyVerificationSession.REMOTE_UNKNOWN_TRANSACTION:
                                return i18nc("Matrix session verification", "The remote party canceled the session verification because it received a message for an unknown session.");
                            case KeyVerificationSession.UNKNOWN_METHOD:
                                return i18nc("Matrix session verification", "The session verification was canceled because NeoChat is unable to handle this verification method.");
                            case KeyVerificationSession.REMOTE_UNKNOWN_METHOD:
                                return i18nc("Matrix session verification", "The remote party canceled the session verification because it is unable to handle this verification method.");
                            case KeyVerificationSession.KEY_MISMATCH:
                                return i18nc("Matrix session verification", "The session verification was canceled because the keys are incorrect.");
                            case KeyVerificationSession.REMOTE_KEY_MISMATCH:
                                return i18nc("Matrix session verification", "The remote party canceled the session verification because the keys are incorrect.\n\n<b>Please log out and log back in, your session is broken/corrupt.</b>");
                            case KeyVerificationSession.USER_MISMATCH:
                                return i18nc("Matrix session verification", "The session verification was canceled because it verifies an unexpected user.");
                            case KeyVerificationSession.REMOTE_USER_MISMATCH:
                                return i18nc("Matrix session verification", "The remote party canceled the session verification because it verifies an unexpected user.");
                            case KeyVerificationSession.INVALID_MESSAGE:
                                return i18nc("Matrix session verification", "The session verification was canceled because we received an invalid message.");
                            case KeyVerificationSession.REMOTE_INVALID_MESSAGE:
                                return i18nc("Matrix session verification", "The remote party canceled the session verification because it received an invalid message.");
                            case KeyVerificationSession.SESSION_ACCEPTED:
                            case KeyVerificationSession.REMOTE_SESSION_ACCEPTED:
                                return i18nc("Matrix session verification", "The session was accepted on a different device");
                            case KeyVerificationSession.MISMATCHED_COMMITMENT:
                                return i18nc("Matrix session verification", "The session verification was canceled because of a mismatched key.");
                            case KeyVerificationSession.REMOTE_MISMATCHED_COMMITMENT:
                                return i18nc("Matrix session verification", "The remote party canceled the session verification because of a mismatched key.");
                            case KeyVerificationSession.MISMATCHED_SAS:
                                return i18nc("Matrix session verification", "The session verification was canceled because the keys do not match.");
                            case KeyVerificationSession.REMOTE_MISMATCHED_SAS:
                                return i18nc("Matrix session verification", "The remote party canceled the session verification because the keys do not match.");
                            default:
                                return i18nc("Matrix session verification", "The session verification was canceled due to an unknown error.");
                            }
                    }
                    default:
                        return "";
                    }
                }
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
            }
        }
    }

    Component {
        id: methodSelector
        QQC2.ItemDelegate {
            id: delegateRoot
            text: i18nc("Matrix session verfication", "Emoji Verification")
            contentItem: Kirigami.TitleSubtitle {
                title: delegateRoot.text
                subtitle: i18nc("Matrix session verification", "Compare a set of emoji on both devices.")
            }
            onClicked: root.session.sendStartSas()
        }
    }

    Component {
        id: sasEmojis
        ColumnLayout {
            QQC2.Label {
                Layout.fillWidth: true
                text: i18nc("Matrix session verification", "Confirm the emoji below are displayed on both devices, in the same order.")
                horizontalAlignment: Text.AlignHCenter
                wrapMode: Text.Wrap
            }
            GridLayout {
                rows: 2
                columns: 4
                Repeater {
                    model: root.session.sasEmojis
                    ColumnLayout {
                        id: delegateRoot
                        required property string emoji
                        required property string description
                        QQC2.Label {
                            Layout.fillWidth: true
                            Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                            Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            font.family: "emoji"
                            font.pointSize: Kirigami.Theme.defaultFont.pointSize * 4
                            text: delegateRoot.emoji
                        }
                        QQC2.Label {
                            Layout.fillWidth: true
                            verticalAlignment: Text.AlignVCenter
                            horizontalAlignment: Text.AlignHCenter
                            text: delegateRoot.description
                        }
                    }
                }
            }
            RowLayout {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignHCenter
                QQC2.Button {
                    Layout.alignment: Qt.AlignBottom
                    text: i18nc("Matrix session verification", "They match")
                    icon.name: "dialog-ok"
                    onClicked: root.session.sendMac()
                }
                QQC2.Button {
                    Layout.alignment: Qt.AlignBottom
                    text: i18nc("Matrix session verification", "They don't match")
                    icon.name: "dialog-cancel"
                    onClicked: root.session.cancelVerification(KeyVerificationSession.MISMATCHED_SAS)
                }
            }
        }
    }

    Loader {
        anchors.fill: parent
        sourceComponent: {
            switch (root.session.state) {
                case KeyVerificationSession.WAITINGFORREADY:
                case KeyVerificationSession.INCOMING:
                case KeyVerificationSession.WAITINGFORMAC:
                case KeyVerificationSession.DONE:
                case KeyVerificationSession.CANCELED:
                    return message;
                case KeyVerificationSession.WAITINGFORVERIFICATION:
                    return sasEmojis
                case KeyVerificationSession.READY:
                    return methodSelector
                default:
                    return undefined;
            }
        }
    }

    footer: QQC2.ToolBar {
        visible: root.session.state === KeyVerificationSession.INCOMING
        QQC2.DialogButtonBox {
            anchors.fill: parent
            Item {
                Layout.fillWidth: true
            }
            QQC2.Button {
                text: i18n("Accept")
                icon.name: "dialog-ok"
                onClicked: root.session.sendReady()
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.AcceptRole
            }
            QQC2.Button {
                text: i18n("Decline")
                icon.name: "dialog-cancel"
                onClicked: root.session.cancelVerification("m.user", "Declined")
                QQC2.DialogButtonBox.buttonRole: QQC2.DialogButtonBox.RejectRole
            }
        }
    }
}
