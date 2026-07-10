const features = [
  {
    icon: '⚡',
    title: 'Controle de alta resposta',
    text: 'A proposta do projeto é entregar uma experiência mais estável e rápida para sistemas embarcados e demonstrações operacionais.'
  },
  {
    icon: '🧠',
    title: 'Arquitetura modular',
    text: 'Cada camada do ecossistema pode ser trabalhada de forma independente, facilitando evolução e integração.'
  },
  {
    icon: '🔒',
    title: 'Foco em robustez',
    text: 'A estrutura foi pensada para lidar com ruído, falhas e mudanças de ambiente sem perder a resposta do sistema.'
  }
];

const modules = [
  {
    title: 'Transmissor / Interface',
    text: 'Recebe entradas do usuário e prepara o fluxo de controle para o restante do sistema com baixa latência.'
  },
  {
    title: 'Cérebro / Core',
    text: 'Processa decisões, compensa movimentos e organiza a lógica de execução do braço e da tração.'
  },
  {
    title: 'Atuadores e laboratório',
    text: 'Servos, motores e sensores transformam a lógica em ação física, com foco em telemetria e observação.'
  }
];

function renderCards(containerId, items) {
  const container = document.getElementById(containerId);
  if (!container) return;

  container.innerHTML = items
    .map(
      (item) => `
        <article class="card">
          <div class="chip">${item.icon || '●'}</div>
          <h3>${item.title}</h3>
          <p>${item.text}</p>
        </article>
      `
    )
    .join('');
}

renderCards('features-grid', features);
renderCards('modules-grid', modules);

document.getElementById('year').textContent = new Date().getFullYear();
