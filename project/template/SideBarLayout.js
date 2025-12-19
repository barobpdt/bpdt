import { useTheme } from 'next-themes';
import { useEffect, useState } from 'react';
import { Sidebar, Menu, MenuItem, SubMenu, menuClasses } from 'react-pro-sidebar';
import { BsBarChartFill } from 'react-icons/bs';
import { RiShoppingCart2Fill } from 'react-icons/ri';
import { Badge } from '~/components/ui/badge';
import { IoAnalyticsSharp } from 'react-icons/io5';
import { CalendarCheck, ChevronRight, Gauge, User2, Users2, X } from 'lucide-react';
import { Button } from '~/components/ui/button';
import { useLocation } from '@remix-run/react';
import sideBarThemes from '../../config/sideBarTheme';
import { useTranslation } from 'react-i18next';

export const handle = { i18n: 'layout' };

const SideBarLayout = ({ collapsed, setCollapsed, toggled, setToggled }) => {
  const { t, i18n } = useTranslation('layout');

  const [sideBarMode, setSidebarMode] = useState('dark');
  const hexToRgba = (hex, alpha) => {
    const r = parseInt(hex.slice(1, 3), 16);
    const g = parseInt(hex.slice(3, 5), 16);
    const b = parseInt(hex.slice(5, 7), 16);

    return `rgba(${r}, ${g}, ${b}, ${alpha})`;
  };
  const { theme, systemTheme } = useTheme();
  const route = useLocation();

  useEffect(() => {
    if (theme === 'system') {
      if (systemTheme === 'system' || systemTheme === undefined) {
        setSidebarMode('dark');
      } else {
        setSidebarMode(systemTheme);
      }
    } else {
      setSidebarMode(theme);
    }
  }, [theme]);

  const menuItemStyles = {
    root: {
      fontSize: '13px',
      fontWeight: 400,
      borderRadius: '8px',
    },
    icon: {
      color: sideBarThemes[sideBarMode].menu.icon,
      [`&.${menuClasses.disabled}`]: {
        color: sideBarThemes[sideBarMode].menu.disabled.color,
      },
      [`&.${menuClasses.active}`]: {
        color: sideBarThemes[sideBarMode].menu.active.color,
      },
    },
    SubMenuExpandIcon: {
      color: '#b6b7b9',
      borderRadius: '8px',
    },
    subMenuContent: ({ level }) => ({
      backgroundColor: level === 0 ? hexToRgba(sideBarThemes[sideBarMode].menu.menuContent, 1) : 'transparent',
    }),
    button: {
      borderRadius: '8px',
      [`&.${menuClasses.disabled}`]: {
        color: sideBarThemes[sideBarMode].menu.disabled.color,
      },
      [`&.${menuClasses.active}`]: {
        color: sideBarThemes[sideBarMode].menu.active.color,
        fontSize: '16px',
        letterSpacing: '0.05em',
      },
      '&:hover': {
        backgroundColor: hexToRgba(sideBarThemes[sideBarMode].menu.hover.backgroundColor, 1),
        color: sideBarThemes[sideBarMode].menu.hover.color,
      },
    },
    label: ({ open }) => ({
      fontWeight: open ? 600 : undefined,
    }),
    active: {
      color: '#ff0000',
    },
  };

  return (
    <div className={`fixed z-30 transition-all duration-300 h-full md:start-0 ${toggled ? 'start-0' : '-start-full'}`}>
      <Sidebar
        collapsed={collapsed}
        backgroundColor={sideBarThemes[sideBarMode].sidebar.backgroundColor}
        onBackdropClick={() => setToggled(false)}
        rootStyles={{
          color: sideBarThemes[sideBarMode].sidebar.color,
          border: 'none',
          borderRadius: '16px',
          overflow: 'hidden',
          height: '100%',
        }}
      >
        <div className="p-4 pb-1 flex justify-between items-center">
          <h4 className={` ${collapsed && 'md:hidden'}`}>MBK</h4>
          <Button className="flex md:hidden" variant="ghost" size="icon" onClick={() => setToggled(!toggled)}>
            <X className={`text-lg transition-transform ${collapsed && 'rotate-180'}`} />
          </Button>
          <Button className="hidden md:flex" variant="ghost" size="icon" onClick={() => setCollapsed(!collapsed)}>
            <ChevronRight
              className={`text-lg transition-transform duration-300 ${
                collapsed && i18n.language === 'fa' && 'rotate-180'
              } ${!collapsed && i18n.language === 'fa' ? 'rotate-0' : !collapsed ? 'rotate-180' : ''} `}
            />
          </Button>
        </div>
        {/* profile */}
        <div className="flex flex-col items-center pb-6">
          <img
            className={`w-14 rounded-full h-14 object-center transition-all duration-300 bg-white dark:bg-slate-900 ${
              collapsed && 'md:w-10 md:h-10'
            }`}
            src="/assets/img/user-placeholder.png"
            alt="user placeholder "
          />
          <div className={`flex flex-col transition-all items-center ${collapsed && 'hidden'}`}>
            <h3 className="mt-2 text-lg">Mohammad Baqer Kohie</h3>
            <p className="mt-0 text-xs text-green-600">Manager</p>
          </div>
        </div>
        <div>
          <div>
            <Menu menuItemStyles={menuItemStyles}>
              <MenuItem href="/" active={route.pathname === '/'} icon={<Gauge className="text-xl" />}>
                {t('sideBarItem.dashboard')}
              </MenuItem>
              <MenuItem
                href="/analytics"
                active={route.pathname.includes('/analytics')}
                icon={<IoAnalyticsSharp className="text-xl" />}
              >
                {t('sideBarItem.analytics')}
              </MenuItem>
              <MenuItem
                href="/calender"
                active={route.pathname.includes('/calender')}
                icon={<CalendarCheck className="text-xl" />}
              >
                {t('sideBarItem.calendar')}
              </MenuItem>
              <MenuItem
                href="/profile"
                active={route.pathname.includes('/profile')}
                icon={<User2 className="text-xl" />}
              >
                {t('sideBarItem.profile')}
              </MenuItem>
              <SubMenu
                active={route.pathname.includes('/users')}
                label={t('sideBarItem.users')}
                icon={<Users2 className="text-lg" />}
              >
                <MenuItem href="/users/customer" active={route.pathname.includes('/users/customer')}>
                  {' '}
                  {t('sideBarItem.customer')}
                </MenuItem>
                <MenuItem href="/users/manager" active={route.pathname.includes('/users/manager')}>
                  {' '}
                  {t('sideBarItem.manager')}
                </MenuItem>
                <MenuItem href="/users/secretary" active={route.pathname.includes('/users/secretary')}>
                  {' '}
                  {t('sideBarItem.secretary')}
                </MenuItem>
              </SubMenu>
              <SubMenu
                active={route.pathname.includes('/charts')}
                label={t('sideBarItem.charts')}
                icon={<BsBarChartFill className="text-lg" />}
                suffix={
                  <Badge variant="destructive" shape="circle">
                    6
                  </Badge>
                }
              >
                <MenuItem href="/charts/piechart" active={route.pathname.includes('/charts/piechart')}>
                  {' '}
                  {t('sideBarItem.pieCharts')}
                </MenuItem>
                <MenuItem href="/charts/linechart" active={route.pathname.includes('/charts/linechart')}>
                  {' '}
                  {t('sideBarItem.lineCharts')}
                </MenuItem>
                <MenuItem href="/charts/barchart" active={route.pathname.includes('/charts/barchart')}>
                  {' '}
                  {t('sideBarItem.barCharts')}
                </MenuItem>
                <MenuItem href="/charts/radarchart" active={route.pathname.includes('/charts/radarchart')}>
                  {' '}
                  {t('sideBarItem.radarCharts')}
                </MenuItem>
                <MenuItem href="/charts/radialchart" active={route.pathname.includes('/charts/radialchart')}>
                  {' '}
                  {t('sideBarItem.radialCharts')}
                </MenuItem>
              </SubMenu>
              <SubMenu
                active={route.pathname.includes('/ecommerce')}
                label={t('sideBarItem.ecommerce')}
                icon={<RiShoppingCart2Fill className="text-lg" />}
              >
                <MenuItem href="/ecommerce/products" active={route.pathname.includes('/ecommerce/products')}>
                  {' '}
                  {t('sideBarItem.product')}
                </MenuItem>
                <MenuItem href="/ecommerce/orders" active={route.pathname.includes('/ecommerce/orders')}>
                  {' '}
                  {t('sideBarItem.orders')}
                </MenuItem>
              </SubMenu>
            </Menu>
          </div>
        </div>
      </Sidebar>
    </div>
  );
};

export default SideBarLayout;